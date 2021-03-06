#include "stdafx.h"
#include "beatmap.h"
#include <vector>
#include <fstream>
#include <sstream>

namespace osu {

	bool split_line(std::wstring line, wchar_t delimiter, std::vector<std::wstring>& result) {
		std::wstringstream input_line(line);
		std::wstring current_line;

		while (std::getline(input_line, current_line, delimiter)) {
			if (!current_line.empty()) {
				result.push_back(current_line);
			}
		}

		return (result.size() >= 2);
	};

	bool Beatmap::GetTimingPointFromOffset(uint32_t offset, TimingPoint& target_point) {
		if (this->timingpoints.size() == 0) {
			return false;
		}

		for (size_t i = this->timingpoints.size(); i-- > 0;) {
			if (this->timingpoints.at(i).offset <= offset) {
				target_point = this->timingpoints.at(i);
				return true;
			}
		}

		target_point = this->timingpoints.front();
		return true;
	}

	bool Beatmap::ParseTimingPoint(std::vector<std::wstring>& values) {
		static float last_ms_per_beat;

		int32_t offset = std::stoi(values.at(0));
		float ms_per_beat = std::stof(values.at(1));
		float velocity = 1.f;

		if (ms_per_beat < 0) {
			velocity = abs(100 / ms_per_beat);

			if (last_ms_per_beat != 0) {
				ms_per_beat = last_ms_per_beat;
			}
		}
		else {
			last_ms_per_beat = ms_per_beat;
		}

		TimingPoint new_timingpoint;

		new_timingpoint.offset = offset;
		new_timingpoint.velocity = velocity;
		new_timingpoint.ms_per_beat = ms_per_beat;

		this->timingpoints.push_back(new_timingpoint);

		return true;
	}

	bool Beatmap::ParseHitObject(std::vector<std::wstring>& values) {
		HitObject new_hitobject;

		new_hitobject.type = std::stoi(values.at(3));
		new_hitobject.x = std::stoi(values.at(0));
		new_hitobject.y = std::stoi(values.at(1));
		new_hitobject.startTime = std::stoi(values.at(2));

		if (new_hitobject.IsCircle()) {
			new_hitobject.endTime = new_hitobject.startTime + 2;
		}
		else if (new_hitobject.IsSpinner()) {
			new_hitobject.endTime = std::stoi(values.at(5));
		}
		else if (new_hitobject.IsSlider()) {
			new_hitobject.repeat = std::stoi(values.at(6));
			new_hitobject.pixelLength = std::stoi(values.at(7));

			TimingPoint object_timingpoint;

			if (this->GetTimingPointFromOffset(new_hitobject.startTime, object_timingpoint)) {
				float px_per_beat = this->slider_multiplier * 100 * object_timingpoint.velocity;
				float beats_num = (new_hitobject.pixelLength * new_hitobject.repeat) / px_per_beat;
				new_hitobject.endTime = static_cast<int>(new_hitobject.startTime + ceil(beats_num * object_timingpoint.ms_per_beat));
			}
		}

		this->hitobjects.push_back(new_hitobject);

		return true;
	}

	bool Beatmap::ParseDifficultySettings(std::wstring difficulty_line) {
		std::vector<std::wstring> values;

		if (split_line(difficulty_line, ':', values)) {
			if (!_wcsicmp(values[0].c_str(), L"slidermultiplier")) {
				this->slider_multiplier = std::stof(values[1]);
			}
		}

		return true;
	}

	bool Beatmap::Parse(std::wstring filename) {
		std::wifstream beatmap_file(filename, std::ifstream::in);

		if (!beatmap_file.good()) {
			return false;
		}

		std::wstring current_line;

		while (std::getline(beatmap_file, current_line)) {
			static std::wstring current_section;

			if (!current_line.empty() && current_line.front() == '[' && current_line.back() == ']') {
				current_section = current_line.substr(1, current_line.length() - 2);
				continue;
			}

			if (current_section.empty()) {
				continue;
			}

			if (!current_section.compare(L"Difficulty")) {
				this->ParseDifficultySettings(current_line);
			}
			else {
				std::vector<std::wstring> values;

				if (!split_line(current_line, L',', values)) {
					continue;
				}

				if (!current_section.compare(L"TimingPoints")) {
					this->ParseTimingPoint(values);
				}
				else if (!current_section.compare(L"HitObjects")) {
					this->ParseHitObject(values);
				}
			}
		}

		return true;
	}
}