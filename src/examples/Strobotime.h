#pragma once
#include "../api/Effect.h"
#include "../api/Params.h"
class Strobotime : public Effect
{
public:
	Strobotime();
	void update() override;
	~Strobotime();
	Param::Ptr dutyCycle;
	ParamRange::Ptr speed;
};

