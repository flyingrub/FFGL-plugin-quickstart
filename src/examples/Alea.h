#pragma once
#include "../api/Source.h"
class Alea : public Source
{
public:
	Alea();
	void update() override;
	~Alea();
private:
	int sides = 5;
};

