#pragma once
#include <FFGLSDK.h>

class Source : public CFreeFrameGLPlugin
{
public:
	Source();
	~Source();

	FFResult InitGL(const FFGLViewportStruct* vp) override;
	FFResult ProcessOpenGL(ProcessOpenGLStruct* pGL) override;
	FFResult DeInitGL() override;

	FFResult SetFloatParameter(unsigned int dwIndex, float value) override;
	float GetFloatParameter(unsigned int index) override;

	const char* GetShortName() override
	{
		static const char* sname = "Red";
		return sname;
	}

private:
	ffglex::FFGLShader shader;
	ffglex::FFGLScreenQuad quad;
};
