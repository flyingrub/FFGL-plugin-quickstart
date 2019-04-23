#pragma once
#include "../api/Effect.h"
class Dissolve : public Effect
{
public:
	Dissolve();
	void update() override;
	~Dissolve();

private:
	float relativeTime = 0;
};
