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

enum PluginType {
	SOURCE, EFFECT, MIXER
};

class Plugin : public CFreeFrameGLPlugin
{
public:
	Plugin(PluginType type);
	~Plugin();
	
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
	PluginType type;
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

	std::string vertexShaderCodeSource = R"(
		#version 410 core
		layout( location = 0 ) in vec4 vPosition;
		layout( location = 1 ) in vec2 vUV;

		out vec2 uv;

		void main()
		{
			gl_Position = vPosition;
			uv = vUV;
		}
	)";
	std::string vertexShaderCodeEffect = R"(
		#version 410 core
		uniform vec2 maxUV;

		layout( location = 0 ) in vec4 vPosition;
		layout( location = 1 ) in vec2 vUV;

		out vec2 uv;

		void main()
		{
			gl_Position = vPosition;
			uv = vUV * maxUV;
		}
	)";
	std::string vertexShaderCodeMixer = R"(
		#version 410 core
		layout( location = 0 ) in vec4 vPosition;
		layout( location = 1 ) in vec2 vUV;

		out vec2 uv;

		void main()
		{
			gl_Position = vPosition;
			uv = vUV;
		}
	)";
};
