#pragma once
#include <algorithm>
#include <random>

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

	class Random {
		std::random_device device;
		std::mt19937 rng;
	public:
		Random() {
			rng = std::mt19937(device());
		}
		
		int getRandomInt(int min, int max) {
			std::uniform_int_distribution distribution(min, max);
			return distribution(rng);
		}
		
		float getRandomFloat(float min, float max) {
			std::uniform_real_distribution distribution(min, max);
			return distribution(rng);
		}
	};
}