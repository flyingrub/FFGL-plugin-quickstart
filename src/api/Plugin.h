#pragma once
#include <FFGLSDK.h>
#include <vector>
#include <string>
#include "../helpers/Audio.h"
#include "../helpers/Utils.h"
#include <chrono>
#include "Params.h";

typedef CFFGLPluginInfo PluginInstance;

struct PluginInfo {
	std::string id, name, description, about;
	int majorVersion = 1;
	int minorVersion = 0;
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

	char* GetParameterDisplay(unsigned int index) override;
	FFResult SetFloatParameter(unsigned int dwIndex, float value) override;
	float GetFloatParameter(unsigned int index) override;
	FFResult SetTextParameter(unsigned int index, const char* value) override;
	char* GetTextParameter(unsigned int index) override;
	void SetSampleRate(unsigned int _sampleRate) override;

	void setFragmentShader(std::string fShader);
	void addParam(Param p);
	void addParam(ParamOption p);
	void addHueColorParam(std::string name);
	void addRGBColorParam(std::string name);
	bool isHueColor(int index);
	bool isRGBColor(int index);
	Param getParam(std::string name);
	ParamOption getParamOption(std::string name);
	ParamText getParamText(std::string name);
	void include(shader::snippet_id snippet);
	void include(std::set<shader::snippet_id> snippets);
	
protected:
	std::string fragmentShader;
	std::vector<Param> params;
	ffglex::FFGLShader shader;
	ffglex::FFGLScreenQuad quad;
	float lastUpdate = 0;
	int frame = 0;
	std::chrono::time_point<std::chrono::high_resolution_clock> t_start = std::chrono::high_resolution_clock::now();
	Audio audio;
	utils::Random random;
	std::set<shader::snippet_id> includedSnippets;

	std::string fragmentShaderCodeStart = R"(
		#version 410 core
		out vec4 fragColor;
		uniform vec2 resolution;
		uniform float time;
		uniform float deltaTime;
		uniform int frame;
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
