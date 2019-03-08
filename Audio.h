#pragma once
#include <FFGLSDK.h>

static const int BUFFER_SIZE = 256;

class Audio
{
public:
	Audio();
	~Audio();
	void update(std::vector<float> fft);

	static int getBufferSize();
};

