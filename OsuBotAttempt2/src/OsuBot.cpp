#include "stdafx.h"
#include "OsuBot.h"
#include <thread>
#include "helpers/Utils.hpp"
#include <iostream>
#include "SignatureScanner.h"
#include <vector>
#include "osu/Beatmap.h"
#include "ReadFileHook.h"
#include <Dwmapi.h>
#include "CodecaveManager.h"

using namespace Utils;
using namespace std;

double lastFilePathCurrentSongTime = 0;
vector<wstring> loadedPreviewOsuPaths;
wstring newDetectedFile;
wstring currentBotFile;
wstring filepathFilter = L".osu";

bool OsuBot::keep_running = true;
double* OsuBot::p_current_song_time;
HMODULE OsuBot::_hModule;
HWND OsuBot::_osuHwnd;
osu::Beatmap OsuBot::_beatmap;

#define OSU_POS_MAX_WIDTH 512
#define OSU_POS_MAX_HEIGHT 384

bool MouseHook(const PMSLLHOOKSTRUCT hookStruct)
{
	cout << "Mouse: (" << hookStruct->pt.x << ", " << hookStruct->pt.y << ")" << endl;
	return false;
}

void ReadFileHookReadFileCallback(wstring filepath)
{
	// Does filepath not contain filter? Ignore this path
	if (filepath.rfind(filepathFilter, filepath.length()) == string::npos) return;

	auto has_already_loaded_path = false;
	for (auto const& loadedPath : loadedPreviewOsuPaths) {
		// Does the list contain the current path?
		if (loadedPath.find(filepath) != string::npos) {
			has_already_loaded_path = true;
			break;
		}
	}
	if (!has_already_loaded_path) {
		loadedPreviewOsuPaths.push_back(wstring(filepath));
	}

	// Is the same as last time
	if (has_already_loaded_path && newDetectedFile.find(filepath) == string::npos) {
		if (lastFilePathCurrentSongTime + static_cast<double>(1) < *OsuBot::p_current_song_time) {
			newDetectedFile = wstring(filepath);
		}
	}
		
	lastFilePathCurrentSongTime = *OsuBot::p_current_song_time;
}

double* OsuBot::locateCurrentSongTime(SignatureScanner scanner)
{
	DWORD address;
	uint8_t pattern[] = { 0xDD, 0x45, 0xEC, 0xDD, 0x1D };
	if (!scanner.scan(pattern, "xxxxx", 5, 5, &address))
	{
		cout << "Memory scan failed." << endl;
		cout << "Exiting application..." << endl;
		cin.get();
		ExitProcess(0);
	}

	DWORD *dw_song_time_ptr_address = reinterpret_cast<DWORD*>(address);
	return reinterpret_cast<double *>(*dw_song_time_ptr_address);
}

bool Is4By3Aspect(const RECT r)
{
	const auto height = r.bottom - r.top;
	const auto width = r.right - r.left;
	const auto x = height * 4 / 3;
	return x < width + 150 || x > width - 150;
}

POINT OsuBot::getScreenPosFromOsuPos(const uint16_t x, const uint16_t y, const uint16_t lastX, const uint16_t lastY, double nextTime, double startTime, double curTime)
{
	const double percentage_diff = (nextTime - curTime) / (nextTime - startTime);
	const uint16_t diff_x = percentage_diff * abs(x - lastX);
	const uint16_t diff_y = percentage_diff * abs(y - lastY);

	RECT rect;
	GetWindowRect(_osuHwnd, &rect);

	if (!Is4By3Aspect(rect))
	{
		cout << "Aspect ratio not supported, trying changing to 4:3" << endl;
		return {0,0};
	}

	const auto width = rect.right - rect.left;
	const auto height = rect.bottom - rect.top;

	rect.left += width * 0.1;
	rect.top += height * 0.1;
	rect.bottom -= height * 0.05;
	rect.right -= width * 0.1;

	const long new_x = rect.left + (x + diff_x) * 100 / OSU_POS_MAX_WIDTH * (rect.right - rect.left) / 100;
	const long new_y = rect.top + (y + diff_y) * 100 / OSU_POS_MAX_HEIGHT * (rect.bottom - rect.top) / 100;
	return { new_x, new_y };
}

POINT OsuBot::getWindowPosFromOsuPos(const uint16_t x, const uint16_t y)
{
	RECT r;
	GetClientRect(_osuHwnd, &r);

	return {
		x * 100 / OSU_POS_MAX_WIDTH * r.right / 100,
		y * 100 / OSU_POS_MAX_HEIGHT * r.bottom / 100,
	};
}

int currentHitObject = 0;
double lastStartTime;
void OsuBot::botLoop(const double st)
{
	// New file was detected
	if (currentBotFile.find(newDetectedFile) == string::npos) {
		currentHitObject = 0;
		currentBotFile = newDetectedFile;
		_beatmap = osu::Beatmap();
		_beatmap.Parse(currentBotFile);

		for (auto const& hitobject : _beatmap.hitobjects) {
			if (!hitobject.IsCircle()) cout << "HitObject at " << hitobject.startTime << " is not a circle and will be ignored" << endl;
			else {
				cout << "Circle (" << hitobject.x << ", " << hitobject.y << ") will be pressed at " << hitobject.startTime << endl;
			}
		}
	}

	if (currentBotFile.empty()) return;

	if (currentHitObject + 1 > _beatmap.hitobjects.size())
	{
		currentBotFile.clear();
		return;
	}

	const auto obj = _beatmap.hitobjects.at(currentHitObject);
	uint16_t last_x = 0;
	uint16_t last_y = 0;
	if (currentHitObject != 0)
	{
		const auto last_obj = _beatmap.hitobjects.at(currentHitObject - 1);
		last_x = last_obj.x;
		last_y = last_obj.y;
	}
	const auto pos = getScreenPosFromOsuPos(obj.x, obj.y, last_x, last_y, obj.startTime, lastStartTime, st);
	cout << "POS " << pos.x << " " << pos.y << endl;
	SetCursorPos(pos.x, pos.y);

	if (obj.startTime < st) {
		currentHitObject++;
		lastStartTime = obj.startTime;
	}
}

void OsuBot::run(const HMODULE hModule, const HWND osuHwnd)
{
	initialize(hModule, osuHwnd);

	const auto sleep_duration = 1ms;
	while (keep_running)
	{
		if (GetAsyncKeyState(VK_END))
		{
			keep_running = false;
			break;
		}
		const auto st = *p_current_song_time;
		botLoop(st);
		this_thread::sleep_for(sleep_duration);
	}

	deinitialize();
}

//CodecaveManager::Codecave Codecave;
//int currentScore;
//DWORD CC_SetScore_Ret;
//__declspec(naked) void CC_SetScore()
//{
//	__asm
//	{
//		// The first thing we must do in our codecave is save 
//		// the return address from the top of the stack
//		pop CC_SetScore_Ret
//
//		// Since we know the current score is in EDX, copy it over into 
//		// our variable
//		MOV currentScore, EAX
//
//		// Remember that we need to preserve registers and the stack!
//		PUSHAD
//		PUSHFD
//	}
//	
//	cout << "New score: " << currentScore << endl;
//
//	__asm
//	{
//		// Restore everything to how it was before
//		POPFD
//		POPAD
//
//		// This is an important part here, we must execute whatever 
//		// code we took out for the codecave.
//		// Also note that we have to use 0x3B9ACA00 for a HEX # 
//		// and not 3B9ACA00, which would be misinterpreted by the compiler.
//		MOV [edx + 0xEC], eax
//
//		// The last thing we must do in our codecave is push 
//		// the return address back onto the stack and then RET back
//		push CC_SetScore_Ret
//		ret
//	}
//}

void OsuBot::initialize(const HMODULE hModule, const HWND hWnd)
{
	_hModule = hModule;
	_osuHwnd = hWnd;

	const SignatureScanner scanner;

	AttachConsole();
	ConsolePrintLn("Attached console.");

	ConsolePrintLn("Scanning memory for current song time...");
	p_current_song_time = locateCurrentSongTime(scanner);
	cout << "Current song time located: " << *p_current_song_time << endl;

	ConsolePrintLn("Placing hooks...");
	ReadFileHook::hook(static_cast<PREAD_FILE_HOOK_READ_FILE_CALLBACK>(ReadFileHookReadFileCallback));
	//MouseHook::Hook(_osuHwnd, (MouseHook::PCallback)MouseHook);
	ConsolePrintLn("Placed hooks.");

	/*DWORD address;
	uint8_t pattern[] = { 0x8B, 0x01, 0x8B, 0x40, 0x2C, 0xFF, 0x50, 0x04, 0x8B, 0x95 };
	scanner.scan(pattern, "xxxxxxxxxx", 10, 13, &address);

	char buffer[100];
	sprintf_s(buffer, "Address: 0x%02x", address);
	cout << buffer << endl;*/

	//Codecave = CodecaveManager::getInstance().place(reinterpret_cast<PBYTE>(address), 6, CC_SetScore);
}

void OsuBot::deinitialize()
{
	ConsolePrintLn("Removing hooks...");
	ReadFileHook::unhook();
	ConsolePrintLn("Removed hooks.");

	//CodecaveManager::getInstance().restore(Codecave);

	DetachConsole();
}
