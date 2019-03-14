#include "Plugin.h"

using namespace ffglex;

static const int FFT_INPUT_INDEX = 0;

Plugin::Plugin()
{
	SetBufferParamInfo(FFT_INPUT_INDEX, "FFT", Audio::getBufferSize(), FF_USAGE_FFT);
}

Plugin::~Plugin()
{
}

FFResult Plugin::InitGL(const FFGLViewportStruct * vp)
{
	resolution[0] = vp->width;
	resolution[1] = vp->height;
	std::string fragmentShaderCode = fragmentShaderCodeStart;
	int i = 0;
	while (i < params.size()) {
		if (isRGBColor(i) || isHueColor(i)) {
			fragmentShaderCode += "uniform vec3 " + params[i].name + ";\n";
			i += 2;
		} else if (params[i].type == FF_TYPE_BOOLEAN) {
			fragmentShaderCode += "uniform bool " + params[i].name + ";\n";
		} else if (params[i].type == FF_TYPE_EVENT) {
			fragmentShaderCode += "uniform bool " + params[i].name + ";\n";
		} else {
			fragmentShaderCode += "uniform float " + params[i].name + ";\n";
		}
		i += 1;
	}
	fragmentShaderCode += fragmentShader;
	init();
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

FFResult Plugin::ProcessOpenGL(ProcessOpenGLStruct * pGL)
{
	ScopedShaderBinding shaderBinding(shader.GetGLID());

	int i = 0;
	while (i < params.size()) {
		if (isRGBColor(i)) {
			std::string name = params[i].name;
			float r = params[i].currentValue;
			float g = params[i + 1].currentValue;
			float b = params[i + 2].currentValue;
			glUniform3f(shader.FindUniform(name.c_str()), r,g,b);
			i += 2;
		} else if (isHueColor(i)) {
			float rgb[3];
			std::string name = params[i].name;
			float hue = params[i].currentValue;
			float saturation = params[i+1].currentValue;
			float brightness = params[i+2].currentValue;
			//we need to make sure the hue doesn't reach 1.0f, otherwise the result will be pink and not red how it should be
			hue = (hue == 1.0f) ? 0.0f : hue; 
			HSVtoRGB(hue, saturation, brightness, rgb[0], rgb[1], rgb[2]);
			glUniform3f(shader.FindUniform(name.c_str()), rgb[0], rgb[1], rgb[2]);
			i += 2;
		} if (params[i].type == FF_TYPE_BOOLEAN) {
			std::string name = params[i].name;
			glUniform1i(shader.FindUniform(name.c_str()), params[i].currentValue);
		} else if (params[i].type == FF_TYPE_EVENT) {
			std::string name = params[i].name;
			glUniform1i(shader.FindUniform(name.c_str()), params[i].currentValue);
		} else {
			std::string name = params[i].name;
			float val = params[i].currentValue;
			glUniform1f(shader.FindUniform(name.c_str()), val);
		}
		i += 1;
	}

	float timeNow = getTicks() / 1000.0f;
	float deltaTime = timeNow - lastUpdate;
	lastUpdate = timeNow;
	glUniform1f(shader.FindUniform("time"), timeNow);
	glUniform1f(shader.FindUniform("deltaTime"), deltaTime);

	glUniform2f(shader.FindUniform("resolution"), resolution[0], resolution[1]);

	std::vector< float > fftData(Audio::getBufferSize());
	const ParamInfo* fftInfo = FindParamInfo(FFT_INPUT_INDEX);
	for (size_t index = 0; index < Audio::getBufferSize(); ++index)
		fftData[index] = fftInfo->elements[index].value;
	audio.update(fftData);
	glUniform1f(shader.FindUniform("audioVolume"), audio.getVolume());
	glUniform1f(shader.FindUniform("audioBass"), audio.getBass());
	glUniform1f(shader.FindUniform("audioMed"), audio.getMed());
	glUniform1f(shader.FindUniform("audioHigh"), audio.getHigh());
	update();
	quad.Draw();

	return FF_SUCCESS;
}

FFResult Plugin::DeInitGL()
{
	clean();
	shader.FreeGLResources();
	quad.Release();
	return FF_SUCCESS;
}

FFResult Plugin::SetFloatParameter(unsigned int index, float value)
{
	if (index == FFT_INPUT_INDEX) return FF_SUCCESS;
	if (index <= params.size()) {
		params[index-1].currentValue = value;
		return FF_SUCCESS;
	} else {
		return FF_FAIL;
	}
}

float Plugin::GetFloatParameter(unsigned int index)
{
	if (0 < index && index <= params.size()) {
		return params[index-1].currentValue;
	} else {
		return 0.0f;
	}
}

void Plugin::setFragmentShader(std::string fShader)
{
	fragmentShader = fShader;
}

void Plugin::addParam(Param param)
{
	params.push_back(param);
	SetParamInfof(params.size(), param.name.c_str(), param.type);
}

void Plugin::addParam(std::string name)
{
	addParam(Param(name));
}

void Plugin::addParam(std::string name, float defaultValue)
{
	addParam(Param(name, FF_TYPE_STANDARD, defaultValue));

}

void Plugin::addBoolParam(std::string name)
{
	addParam(Param(name, FF_TYPE_BOOLEAN));
}

void Plugin::addButtonParam(std::string name)
{
	addParam(Param(name, FF_TYPE_EVENT));
}

void Plugin::addOptionParam(Param param)
{
	params.push_back(param);
	int index = params.size();
	SetOptionParamInfo(index, param.name.c_str(), param.options.size(), param.currentValue);
	
	for (int i = 0; i < param.options.size(); i++) {
		SetParamElementInfo(index, i, param.options[i].name.c_str(), param.options[i].value);
	}
}

void Plugin::addHueColorParam(std::string name)
{
	addParam(Param(name, FF_TYPE_HUE,0.5));
	addParam(Param(name, FF_TYPE_SATURATION,1.0));
	addParam(Param(name, FF_TYPE_BRIGHTNESS,1.0));
}

void Plugin::addRGBColorParam(std::string name)
{
	addParam(Param(name, FF_TYPE_RED));
	addParam(Param(name, FF_TYPE_GREEN));
	addParam(Param(name, FF_TYPE_BLUE));
}

bool Plugin::isHueColor(int index)
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

bool Plugin::isRGBColor(int index)
{
	bool enoughSpace = index + 2 < params.size();
	if (!enoughSpace) return false;
	bool isColorType =
		params[index].type == FF_TYPE_RED &&
		params[index + 1].type == FF_TYPE_GREEN &&
		params[index + 2].type == FF_TYPE_BLUE;
	if (!isColorType) return false;

	bool isSameName =
		params[index].name.compare(params[index + 1].name) == 0 &&
		params[index].name.compare(params[index + 2].name) == 0;

	return isSameName;
}

Param Plugin::getParam(std::string name)
{
	for (int i = 0; i < params.size(); i++) {
		if (params[i].name.compare(name) == 0) return params[i];
	}
	return Param();
}
