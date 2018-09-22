#pragma once

#include "stdafx.h"
#include <vector>

namespace osu {
	enum HitObjectType : uint8_t {
		HITOBJECT_CIRCLE = 1,
		HITOBJECT_SLIDER = 2,
		HITOBJECT_NEW_COMBO = 4,
		HITOBJECT_SPINNER = 8
	};

	struct HitObject {
		uint8_t type;
		uint16_t x;
		uint16_t y;
		int32_t startTime;
		int32_t endTime;
		uint32_t repeat;
		uint32_t pixelLength;

		bool IsCircle() const;

		bool IsSlider() const;

		bool IsSpinner() const;
	};
}