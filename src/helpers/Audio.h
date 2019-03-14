#pragma once
#include <FFGLSDK.h>
#include "Utils.h"

static const int BUFFER_SIZE = 512;

class Audio
{
public:
	Audio();
	~Audio();
	void update(std::vector<float> fft);
	float getVolume();
	float getBass();
	float getMed();
	float getHigh();
	float getVolumeFromTo(int fromFreq, int toFreq);
	float toDb(float rms);
	void setSmoothness(float smoothness);

	static int getBufferSize();
private:
	std::vector<float> fft;
	utils::SmoothValue vol, bass, med, high;
	float freqMin, freqMax, freqBinStep;
};

