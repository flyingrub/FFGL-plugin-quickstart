#pragma once
#include <FFGLSDK.h>
#include <vector>
#include <string>
#include "Audio.h"

class Param {
public:
	std::string name;
	FFUInt32 type;
	float currentValue;

	Param() : Param ("", FF_TYPE_STANDARD) {}

	Param(std::string name) : Param(name, FF_TYPE_STANDARD) {}

	Param(std::string name, FFUInt32 type) : Param(name, type, 0.0f) {}

	Param(std::string name, FFUInt32 type, float currentVal) :
		name(name),
		type(type),
		currentValue(currentVal) {}
};

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
	
	void setFragmentShader(std::string fShader);
	void addParam(Param p);
	void addColorParam(std::string name);
	bool isColor(int index);
private:
	std::string fragmentShader;
	std::vector<Param> params;
	ffglex::FFGLShader shader;
	ffglex::FFGLScreenQuad quad;
	float lastUpdate;
	Audio audio;
};
