#include "Audio.h"
#include <cmath>

Audio::Audio()
{
}

Audio::~Audio()
{
}

void Audio::update(std::vector<float> _fft)
{
	fft = _fft;
}

float Audio::getCurrentVolume()
{
	float res = 0;
	for (int i = 0; i < Audio::getBufferSize(); i++) {
		res += fft[i] * fft[i];
	}
	res /= (float) Audio::getBufferSize();
	float rms = sqrt(res);
	float db = 20 * std::log10(rms);
	db = utils::map(db, -50, -5, 0, 1);
	if (std::isinf(db)) db = 0;
	currentVol.update(db);
	return currentVol.getValue();
}

int Audio::getBufferSize()
{
	return BUFFER_SIZE;
}
