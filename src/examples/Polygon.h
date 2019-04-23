#pragma once
#include "../api/Source.h"
class PolygonRepeat : public Source
{
public:
	PolygonRepeat();
	void update() override;
	~PolygonRepeat();

private:
	float relativeTime         = 0;
	float relativeRotationTime = 0;
};
