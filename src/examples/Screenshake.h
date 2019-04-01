#pragma once
#include "../api/Effect.h"
class Screenshake : public Effect
{
public:
	Screenshake();
	void update() override;
	~Screenshake();
};

