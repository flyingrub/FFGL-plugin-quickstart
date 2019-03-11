#pragma once
#include "Plugin.h"
class Effect : public Plugin
{
public:
	Effect();
	~Effect();
	FFResult InitGL(const FFGLViewportStruct * vp) override;
	FFResult ProcessOpenGL(ProcessOpenGLStruct * pGL) override;
private:
};

