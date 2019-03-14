#pragma once
#include <FFGLSDK.h>
#include <vector>
#include <string>
#include "../helpers/Audio.h"
#include "../helpers/Utils.h"

typedef CFFGLPluginInfo PluginInstance;

struct PluginInfo {
	std::string id, name, description, about;
	int majorVersion = 1;
	int minorVersion = 0;
};

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

struct OptionParam {
	Param p;
	std::vector<std::string> options;
};

class Plugin : public CFreeFrameGLPlugin
{
public:
	Plugin();
	~Plugin();

	template<typename PluginType>
	static PluginInstance createPlugin(PluginInfo infos, FFUInt32 type);

	FFResult InitGL(const FFGLViewportStruct* vp) override;
	FFResult ProcessOpenGL(ProcessOpenGLStruct* pGL) override;
	FFResult DeInitGL() override;
	virtual void init() {};
	virtual void update() {};
	virtual void clean() {};

	FFResult SetFloatParameter(unsigned int dwIndex, float value) override;
	float GetFloatParameter(unsigned int index) override;
	
	void setFragmentShader(std::string fShader);
	void addParam(Param p);
	void addParam(std::string name);
	void addParam(std::string name, float defaultValue);
	void addBoolParam(std::string name);
	void addButtonParam(std::string name);
	void addOptionParam()
	void addHueColorParam(std::string name);
	void addRGBColorParam(std::string name);
	bool isHueColor(int index);
	bool isRGBColor(int index);
	Param getParam(std::string name);
	
protected:
	std::string fragmentShader;
	std::vector<Param> params;
	ffglex::FFGLShader shader;
	ffglex::FFGLScreenQuad quad;
	float lastUpdate;
	Audio audio;
	float resolution[2];
	utils::Random random;

	std::string fragmentShaderCodeStart = R"(
		#version 410 core
		out vec4 fragColor;
		uniform vec2 resolution;
		uniform float time;
		uniform float deltaTime;
		uniform float audioVolume;
		uniform float audioBass;
		uniform float audioMed;
		uniform float audioHigh;
	)";
	std::string vertexShaderCode;
};

template<typename PluginType>
inline PluginInstance Plugin::createPlugin(PluginInfo infos, FFUInt32 type)
{
	return PluginInstance(
		PluginFactory< PluginType >,	// Create method
		infos.id.c_str(),				// Plugin unique ID
		infos.name.c_str(),				// Plugin name
		2,								// API major version number
		1,								// API minor version number
		infos.majorVersion,				// Plugin major version number
		infos.minorVersion,				// Plugin minor version number
		type,							// Plugin type
		infos.description.c_str(),		// Plugin description
		infos.about.c_str()				// About
	);
}
