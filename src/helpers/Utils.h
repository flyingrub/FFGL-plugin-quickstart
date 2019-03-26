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

namespace shader {
// float random( uvec2 x )
// Hardware indepedant random
// Pass it gl_FragCoord to get a random nomber for each pixel

// float random()
// Different random number for each pixel and frame
static const std::string random = R"(
float random( uvec2 x )
{
    uvec2 q = 1103515245U * ( (x>>1U) ^ (x.yx   ) );
    uint  n = 1103515245U * ( (q.x  ) ^ (q.y>>3U) );
    return float(n) * (1.0/float(0xffffffffU));
}

float random() {
    uvec2 p = uvec2(gl_FragCoord) + uint(resolution.x) * uint(resolution.y) * uint(frame);
	return random(p);
}
)";

// Remap a value between 0.0 and 1.0
static const std::string map = R"(
float map( float t, float a, float b ) {
	return clamp( (t - a) / (b - a), 0.0, 1.0 );
}

vec2 map( vec2 t, vec2 a, vec2 b ) {
	return clamp( (t - a) / (b - a), 0.0, 1.0 );
}
)";

}