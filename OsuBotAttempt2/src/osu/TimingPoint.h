#pragma once
#include <vector>

namespace osu {
	struct TimingPoint {
		uint32_t offset;
		float velocity;
		float ms_per_beat;
	};
}