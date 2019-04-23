#pragma once
#include "../api/Source.h"
class Triskel : public Source
{
public:
	Triskel();
	void update();
	~Triskel();
private:
	float relativeTime = 0;
};
