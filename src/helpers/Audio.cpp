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
	float currentVolStore = 0;
	float bassStore = 0;
	float medStore = 0;
	float highStore = 0;

	for (int i = 0; i < Audio::getBufferSize(); i++) {
		float bin = fft[i] * fft[i];
		
		if (i < 10) bassStore += bin;
		if (240 < i && i < 260) medStore += bin;
		if (Audio::getBufferSize() - 20 < i) highStore += bin;
		
		currentVolStore += bin;
	}
	currentVolStore /= (float)Audio::getBufferSize();
	currentVolStore = sqrt(currentVolStore);
	vol.update(toDb(currentVolStore));

	bassStore /= 10.0f;
	bassStore = sqrt(bassStore);
	bass.update(toDb(bassStore));

	medStore /= 20.0f;
	medStore = sqrt(medStore);
	med.update(toDb(medStore));

	highStore /= 20.0f;
	highStore = sqrt(highStore);
	high.update(toDb(highStore));
}

float Audio::getVolume()
{
	
	return vol.getValue();
}

float Audio::getBass()
{
	return bass.getValue();
}

float Audio::getMed()
{
	return med.getValue();
}

float Audio::getHigh()
{
	return high.getValue();
}

float Audio::getVolumeFromTo(int fromFreq, int toFreq)
{
	return 0.0f;
}

float Audio::toDb(float rms)
{
	float db = 20 * std::log10(rms);
	db = utils::map(db, -50, -3, 0, 1);
	if (std::isinf(db)) db = 0;
	return db;
}

void Audio::setSmoothness(float smoothness)
{
	vol.setSmoothness(smoothness);
	bass.setSmoothness(smoothness);
	med.setSmoothness(smoothness);
	high.setSmoothness(smoothness);
}

int Audio::getBufferSize()
{
	return BUFFER_SIZE;
}
