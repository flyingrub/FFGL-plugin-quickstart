#pragma once
#include "../api/Effect.h"
class BarrelBlur : public Effect
{
public:
	BarrelBlur();
	void update() override;
	~BarrelBlur();
};

