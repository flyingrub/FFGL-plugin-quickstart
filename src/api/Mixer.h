#pragma once
#include "Plugin.h"
class Mixer :
	public Plugin
{
public:
	Mixer();
	~Mixer();
	FFResult InitGL(const FFGLViewportStruct * vp) override;
	FFResult ProcessOpenGL(ProcessOpenGLStruct * pGL) override;
};

