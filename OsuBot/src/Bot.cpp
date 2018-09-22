#include "stdafx.h"
#include "Bot.h"
#include <thread>
#include "Utils.hpp"
#include "Version.h"

bool keepRunning = true;
auto loopDelay = std::chrono::milliseconds(1);

namespace osubot
{
	void Deinitialize()
	{
		
	}

	void Initialize()
	{
		Utils::AttachConsole();
		Utils::ConsolePrint("OsuBot %s by %s\n", VERSION, AUTHOR);
	}

	void Run()
	{
		Initialize();

		while (keepRunning)
		{
			std::this_thread::sleep_for(loopDelay);
		}

		Deinitialize();
	}
}