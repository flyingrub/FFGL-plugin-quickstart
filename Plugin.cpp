#include "Plugin.h"
using namespace ffglex;

static const int FFT_INPUT_INDEX = 0;

Plugin::Plugin(PluginType type)
{
	if (type == PluginType::SOURCE) {
		SetMinInputs(0);
		SetMaxInputs(0);
		SetBufferParamInfo(FFT_INPUT_INDEX, "FFT", Audio::getBufferSize(), FF_USAGE_FFT);
		fragmentShaderCodeStart += R"(
			in vec2 uv;
		)";
	} else if (type == PluginType::EFFECT) {
		SetMinInputs(1);
		SetMaxInputs(1);
		SetBufferParamInfo(FFT_INPUT_INDEX, "FFT", Audio::getBufferSize(), FF_USAGE_FFT);
		fragmentShaderCodeStart += R"(
			in vec2 uv;
			uniform vec2 MaxUV;

			uniform sampler2D inputTexture;
		)";
	} else  if (type == PluginType::MIXER) {
		SetMinInputs(2);
		SetMaxInputs(2);
		fragmentShaderCodeStart += R"(
			uniform sampler2D textureDest;
			uniform sampler2D textureSrc;
			//the value defined by the slider to switch between the two images
			uniform float mixVal;

			in vec2 uvDest;
			in vec2 uvSrc;
		)";
	}

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
		if (isColor(i)) {
			fragmentShaderCode += "uniform vec3 " + params[i].name + ";\n";
			i += 3;
		} else {
			fragmentShaderCode += "uniform float " + params[i].name + ";\n";
			i += 1;
		}
	}
	fragmentShaderCode += fragmentShader;
	std::string vertexShaderCode;
	if (type == PluginType::MIXER) {
		vertexShaderCode = vertexShaderCodeMixer;
	} else if (type == PluginType::SOURCE) {
		vertexShaderCode = vertexShaderCodeSource;
	} else if (type == PluginType::EFFECT) {
		vertexShaderCode = vertexShaderCodeEffect;
	}

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

	if (type == PluginType::EFFECT) {
		glUniform1i(shader.FindUniform("inputTexture"), 0);
	}
	return CFreeFrameGLPlugin::InitGL(vp);

}

FFResult Plugin::ProcessOpenGL(ProcessOpenGLStruct * pGL)
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

	glUniform2f(shader.FindUniform("resolution"), resolution[0], resolution[1]);

	std::vector< float > fftData(Audio::getBufferSize());
	const ParamInfo* fftInfo = FindParamInfo(FFT_INPUT_INDEX);
	for (size_t index = 0; index < Audio::getBufferSize(); ++index)
		fftData[index] = fftInfo->elements[index].value;
	audio.update(fftData);
	glUniform1f(shader.FindUniform("audioVolume"), audio.getCurrentVolume());

	if (type == PluginType::MIXER) {
		//TODO
	} else if (type == PluginType::EFFECT) {
		FFGLTextureStruct& Texture = *(pGL->inputTextures[0]);

		//The input texture's dimension might change each frame and so might the content area.
		//We're adopting the texture's maxUV using a uniform because that way we dont have to update our vertex buffer each frame.
		FFGLTexCoords maxCoords = GetMaxGLTexCoords(Texture);
		glUniform2f(shader.FindUniform("maxUV"), maxCoords.s, maxCoords.t);
		ScopedSamplerActivation activateSampler(0);
		Scoped2DTextureBinding textureBinding(Texture.Handle);
	}

	quad.Draw();

	return FF_SUCCESS;
}

FFResult Plugin::DeInitGL()
{
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

void Plugin::addColorParam(std::string name)
{
	addParam(Param(name, FF_TYPE_HUE));
	addParam(Param(name, FF_TYPE_SATURATION,1.0));
	addParam(Param(name, FF_TYPE_BRIGHTNESS,1.0));
}

bool Plugin::isColor(int index)
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
