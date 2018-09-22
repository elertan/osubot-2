#pragma once
#include "stdafx.h"
#include "HitObject.h"
#include "TimingPoint.h"

namespace osu {
	class Beatmap {
	private:
		float slider_multiplier;

		bool GetTimingPointFromOffset(uint32_t offset, TimingPoint& target_point);
		bool ParseTimingPoint(std::vector<std::wstring>& values);
		bool ParseHitObject(std::vector<std::wstring>& values);
		bool ParseDifficultySettings(std::wstring difficulty_line);
	public:
		bool Parse(std::wstring filename);

		std::vector<HitObject> hitobjects;
		std::vector<TimingPoint> timingpoints;
	};
}