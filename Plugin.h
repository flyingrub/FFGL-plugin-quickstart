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

class Plugin : public CFreeFrameGLPlugin
{
public:
	Plugin();
	~Plugin();
	
	FFResult InitGL(const FFGLViewportStruct* vp) override;
	FFResult ProcessOpenGL(ProcessOpenGLStruct* pGL) override;
	FFResult DeInitGL() override;

	FFResult SetFloatParameter(unsigned int dwIndex, float value) override;
	float GetFloatParameter(unsigned int index) override;
	
	void setFragmentShader(std::string fShader);
	void addParam(Param p);
	void addParam(std::string name);
	void addHueColorParam(std::string name);
	void addRGBColorParam(std::string name);
	bool isHueColor(int index);
	bool isRGBColor(int index);
protected:
	std::string fragmentShader;
	std::vector<Param> params;
	ffglex::FFGLShader shader;
	ffglex::FFGLScreenQuad quad;
	float lastUpdate;
	Audio audio;
	float resolution[2];

	std::string fragmentShaderCodeStart = R"(
		#version 410 core
		out vec4 fragColor;
		uniform vec2 resolution;
		uniform float time;
		uniform float deltaTime;
		uniform float audioVolume;
	)";
	std::string vertexShaderCode;
};
