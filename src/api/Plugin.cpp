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
			float r = params[i].value;
			float g = params[i + 1].value;
			float b = params[i + 2].value;
			glUniform3f(shader.FindUniform(name.c_str()), r,g,b);
			i += 2;
		} else if (isHueColor(i)) {
			float rgb[3];
			std::string name = params[i].name;
			float hue = params[i].value;
			float saturation = params[i+1].value;
			float brightness = params[i+2].value;
			//we need to make sure the hue doesn't reach 1.0f, otherwise the result will be pink and not red how it should be
			hue = (hue == 1.0f) ? 0.0f : hue; 
			HSVtoRGB(hue, saturation, brightness, rgb[0], rgb[1], rgb[2]);
			glUniform3f(shader.FindUniform(name.c_str()), rgb[0], rgb[1], rgb[2]);
			i += 2;
		} if (params[i].type == FF_TYPE_BOOLEAN) {
			std::string name = params[i].name;
			glUniform1i(shader.FindUniform(name.c_str()), params[i].value);
		} else if (params[i].type == FF_TYPE_EVENT) {
			std::string name = params[i].name;
			glUniform1i(shader.FindUniform(name.c_str()), params[i].value);
		} else {
			std::string name = params[i].name;
			float val = params[i].getValue();
			glUniform1f(shader.FindUniform(name.c_str()), val);
		}
		i += 1;
	}
	frame++;
	auto t_now = std::chrono::high_resolution_clock::now();
	float timeNow = std::chrono::duration<double, std::milli>(t_now - t_start).count() / 1000.0f;
	float deltaTime = timeNow - lastUpdate;
	lastUpdate = timeNow;
	glUniform1f(shader.FindUniform("time"), timeNow);
	glUniform1f(shader.FindUniform("deltaTime"), deltaTime);
	glUniform1i(shader.FindUniform("frame"), frame);
	glUniform2f(shader.FindUniform("resolution"), currentViewport.width, currentViewport.height);

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

char * Plugin::GetParameterDisplay(unsigned int index)
{
	/**
	 * We're not returning ownership over the string we return, so we have to somehow guarantee that
	 * the lifetime of the returned string encompasses the usage of that string by the host. Having this static
	 * buffer here keeps previously returned display string alive until this function is called again.
	 * This happens to be long enough for the hosts we know about.
	 */
	static char displayValueBuffer[15];
	memset(displayValueBuffer, 0, sizeof(displayValueBuffer));
	if (0 < index && index <= params.size()) {
		sprintf(displayValueBuffer, "%f", params[index - 1].getValue());
		return displayValueBuffer;
	} else {
		return CFreeFrameGLPlugin::GetParameterDisplay(index);
	}
}

FFResult Plugin::SetFloatParameter(unsigned int index, float value)
{
	if (index == FFT_INPUT_INDEX) return FF_SUCCESS;
	if (index <= params.size()) {
		params[index-1].value = value;
		return FF_SUCCESS;
	} else {
		return FF_FAIL;
	}
}

float Plugin::GetFloatParameter(unsigned int index)
{
	if (0 < index && index <= params.size()) {
		return params[index-1].value;
	} else {
		return 0.0f;
	}
}

FFResult Plugin::SetTextParameter(unsigned int index, const char * value)
{
	if (0 < index && index <= params.size()) {
		params[index - 1].text = value == nullptr ? "" : value;
		return FF_SUCCESS;
	} else {
		return FF_FAIL;
	}
}

char * Plugin::GetTextParameter(unsigned int index)
{
	if (0 < index && index <= params.size()) {
		return const_cast<char*>(params[index - 1].text.c_str());
	} else {
		return "";
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
	SetOptionParamInfo(index, param.name.c_str(), param.options.size(), param.value);
	
	for (int i = 0; i < param.options.size(); i++) {
		SetParamElementInfo(index, i, param.options[i].name.c_str(), param.options[i].value);
	}
}

void Plugin::addTextParam(std::string name)
{
	addParam(Param(name, FF_TYPE_TEXT));
}

void Plugin::addHueColorParam(std::string name)
{
	addParam(Param(name + "_hue", FF_TYPE_HUE,0.5));
	addParam(Param(name + "_saturation", FF_TYPE_SATURATION,1.0));
	addParam(Param(name + "_brighthness", FF_TYPE_BRIGHTNESS,1.0));
}

void Plugin::addRGBColorParam(std::string name)
{
	addParam(Param(name + "_red", FF_TYPE_RED));
	addParam(Param(name + "_green", FF_TYPE_GREEN));
	addParam(Param(name + "_blue", FF_TYPE_BLUE));
}

bool Plugin::isHueColor(int index)
{
	bool enoughSpace = index + 2 < params.size();
	if (!enoughSpace) return false;
	bool isColorType =
		params[index].type == FF_TYPE_HUE &&
		params[index + 1].type == FF_TYPE_SATURATION &&
		params[index + 2].type == FF_TYPE_BRIGHTNESS;

	return isColorType;
}

bool Plugin::isRGBColor(int index)
{
	bool enoughSpace = index + 2 < params.size();
	if (!enoughSpace) return false;
	bool isColorType =
		params[index].type == FF_TYPE_RED &&
		params[index + 1].type == FF_TYPE_GREEN &&
		params[index + 2].type == FF_TYPE_BLUE;
	if (isColorType) return false;

	return isColorType;
}

Param Plugin::getParam(std::string name)
{
	for (int i = 0; i < params.size(); i++) {
		if (params[i].name.compare(name) == 0) return params[i];
	}
	return Param();
}
