#pragma once
#include "../api/Source.h"
class PolygonRepeat : public Source
{
public:
	PolygonRepeat();
	void update() override;
	~PolygonRepeat();
};

