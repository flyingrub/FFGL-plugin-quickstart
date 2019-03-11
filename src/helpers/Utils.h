#pragma once
#include <algorithm>

namespace utils {
	inline float map(float value, float low, float high, float newLow, float newHigh) {
		float res = newLow + (value - low) * (newHigh - newLow) / (high - low);
		return std::clamp(res, newLow, newHigh);
	}

	class SmoothValue {

	public:
		SmoothValue() {}
		void setSmoothness(float smooth) {
			smoothness = smooth;
		}
		void update(float val) {
			value *= smoothness;
			value += (1-smoothness) * val;
		}
		float getValue() {
			return value;
		}
	private:
		float smoothness = 0.80f;
		float value = 0.0f;
	};
}