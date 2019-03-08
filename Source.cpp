#include "Source.h"
using namespace ffglex;

static const int FFT_INPUT_INDEX = 0;

static std::string vertexShaderCode = R"(#version 410 core
layout( location = 0 ) in vec4 vPosition;
layout( location = 1 ) in vec2 vUV;

out vec2 uv;

void main()
{
	gl_Position = vPosition;
	uv = vUV;
}
)";

static std::string fragmentShaderCodeStart = R"(
#version 410 core
in vec2 uv;
out vec4 fragColor;
uniform float time;
uniform float deltaTime;
)";

Source::Source()
{
	SetMinInputs(0);
	SetMaxInputs(0);
	SetBufferParamInfo(FFT_INPUT_INDEX, "FFT", Audio::getBufferSize(), FF_USAGE_FFT);
}

Source::~Source()
{
}

FFResult Source::InitGL(const FFGLViewportStruct * vp)
{
	std::string fragmentShaderCode = fragmentShaderCodeStart;
	int i = 0;
	while (i < params.size()) {
		if (isColor(i)) {
			fragmentShaderCode += "uniform vec3 " + params[i].name + ";\n";
			i += 3;
		} else {
			fragmentShaderCode += "uniform float " + params[i].name + ";\n";
			i += 1;
		}
	}
	fragmentShaderCode += fragmentShader;

	if (!shader.Compile(vertexShaderCode, fragmentShaderCode))
	{
		DeInitGL();
		return FF_FAIL;
	}
	if (!quad.Initialise())
	{
		DeInitGL();
		return FF_FAIL;
	}
	return CFreeFrameGLPlugin::InitGL(vp);

}

FFResult Source::ProcessOpenGL(ProcessOpenGLStruct * pGL)
{
	ScopedShaderBinding shaderBinding(shader.GetGLID());
	int i = 0;
	while (i < params.size()) {
		if (isColor(i)) {
			float rgb[3];
			std::string name = params[i].name;
			float hue = params[i].currentValue;
			float saturation = params[i+1].currentValue;
			float brightness = params[i+2].currentValue;
			//we need to make sure the hue doesn't reach 1.0f, otherwise the result will be pink and not red how it should be
			hue = (hue == 1.0f) ? 0.0f : hue; 
			HSVtoRGB(hue, saturation, brightness, rgb[0], rgb[1], rgb[2]);
			glUniform3f(shader.FindUniform(name.c_str()), rgb[0], rgb[1], rgb[2]);
			i += 3;
		} else {
			std::string name = params[i].name;
			float val = params[i].currentValue;
			glUniform1f(shader.FindUniform(name.c_str()), val);
			i += 1;
		}
	}
	float timeNow = getTicks() / 1000.0f;
	float deltaTime = timeNow - lastUpdate;
	lastUpdate = timeNow;
	glUniform1f(shader.FindUniform("time"), timeNow);
	glUniform1f(shader.FindUniform("deltaTime"), deltaTime);

	std::vector< float > fftData(Audio::getBufferSize());
	const ParamInfo* fftInfo = FindParamInfo(FFT_INPUT_INDEX);
	for (size_t index = 0; index < Audio::getBufferSize(); ++index)
		fftData[index] = fftInfo->elements[index].value;
	audio.update(fftData);


	quad.Draw();

	return FF_SUCCESS;
}

FFResult Source::DeInitGL()
{
	shader.FreeGLResources();
	quad.Release();
	return FF_SUCCESS;
}

FFResult Source::SetFloatParameter(unsigned int index, float value)
{
	if (index == FFT_INPUT_INDEX) return FF_SUCCESS;
	if (index <= params.size()) {
		params[index-1].currentValue = value;
		return FF_SUCCESS;
	} else {
		return FF_FAIL;
	}
}

float Source::GetFloatParameter(unsigned int index)
{
	if (0 < index && index <= params.size()) {
		return params[index-1].currentValue;
	} else {
		return 0.0f;
	}
}

void Source::setFragmentShader(std::string fShader)
{
	fragmentShader = fShader;
}

void Source::addParam(Param param)
{
	params.push_back(param);
	SetParamInfof(params.size(), param.name.c_str(), param.type);
}

void Source::addColorParam(std::string name)
{
	addParam(Param(name, FF_TYPE_HUE));
	addParam(Param(name, FF_TYPE_SATURATION,1.0));
	addParam(Param(name, FF_TYPE_BRIGHTNESS,1.0));
}

bool Source::isColor(int index)
{
	bool enoughSpace = index + 2 < params.size();
	if (!enoughSpace) return false;
	bool isColorType =
		params[index].type == FF_TYPE_HUE &&
		params[index + 1].type == FF_TYPE_SATURATION &&
		params[index + 2].type == FF_TYPE_BRIGHTNESS;
	if (!isColorType) return false;

	bool isSameName =
		params[index].name.compare(params[index + 1].name) == 0 &&
		params[index].name.compare(params[index + 2].name) == 0;

	return isSameName;
}
