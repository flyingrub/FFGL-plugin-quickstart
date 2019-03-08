#pragma once
#include <FFGLSDK.h>
#include "Utils.h"

static const int BUFFER_SIZE = 256;

class Audio
{
public:
	Audio();
	~Audio();
	void update(std::vector<float> fft);
	float getCurrentVolume();
	float getCurrentVolumeFromTo(int fromFreq, int toFreq);

	static int getBufferSize();
private:
	std::vector<float> fft;
	utils::SmoothValue currentVol;
};

