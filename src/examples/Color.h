#pragma once
#include "../api/Source.h"
class Color : public Source
{
public:
	Color();
	void update() override;
	~Color();
private:
	int sides = 5;
};

