#include "Effect.h"
using namespace ffglex;



Effect::Effect()
{
	SetMinInputs(1);
	SetMaxInputs(1);
	fragmentShaderCodeStart += R"(
		in vec2 uv;
		uniform vec2 MaxUV;

		uniform sampler2D inputTexture;
	)";
	vertexShaderCode = R"(
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
}


Effect::~Effect()
{
}

FFResult Effect::InitGL(const FFGLViewportStruct * vp)
{
	FFResult res = Plugin::InitGL(vp);
	glUniform1i(shader.FindUniform("inputTexture"), 0);
	return res;
}

FFResult Effect::ProcessOpenGL(ProcessOpenGLStruct * pGL)
{
	ScopedShaderBinding shaderBinding(shader.GetGLID());

	FFGLTextureStruct& Texture = *(pGL->inputTextures[0]);

	//The input texture's dimension might change each frame and so might the content area.
	//We're adopting the texture's maxUV using a uniform because that way we dont have to update our vertex buffer each frame.
	FFGLTexCoords maxCoords = GetMaxGLTexCoords(Texture);
	glUniform2f(shader.FindUniform("maxUV"), maxCoords.s, maxCoords.t);
	ScopedSamplerActivation activateSampler(0);
	Scoped2DTextureBinding textureBinding(Texture.Handle);
	return Plugin::ProcessOpenGL(pGL);
}