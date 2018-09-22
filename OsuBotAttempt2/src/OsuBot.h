#pragma once
#include <Windows.h>
#include "SignatureScanner.h"
#include "osu/Beatmap.h"

class OsuBot
{
public:
	static bool keep_running;
	static double* p_current_song_time;
	static void run(const HMODULE hModule, const HWND osuHwnd);
private:
	OsuBot() = delete;
	static HMODULE _hModule;
	static HWND _osuHwnd;
	static osu::Beatmap _beatmap;
	static double* locateCurrentSongTime(SignatureScanner scanner);
	static POINT getScreenPosFromOsuPos(const uint16_t x, const uint16_t y, const uint16_t lastX, const uint16_t lastY, double nextTime, double startTime, double
	                                    curTime);
	static POINT getWindowPosFromOsuPos(const uint16_t x, const uint16_t);
	static void initialize(const HMODULE hModule, const HWND hWnd);
	static void deinitialize();
	static void botLoop(double st);
};

