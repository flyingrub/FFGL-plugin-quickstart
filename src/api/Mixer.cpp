#include "Mixer.h"
using namespace ffglex;

Mixer::Mixer()
{
	SetMinInputs(2);
	SetMaxInputs(2);
	fragmentShaderCodeStart += R"(
		uniform sampler2D textureDest;
		uniform sampler2D textureSrc;

		in vec2 uvDest;
		in vec2 uvSrc;
	)";
	vertexShaderCode = R"(#version 410 core
		uniform vec2 maxUVDest;
		uniform vec2 maxUVSrc;

		layout( location = 0 ) in vec4 vPosition;
		layout( location = 1 ) in vec2 vUV;

		out vec2 uvDest;
		out vec2 uvSrc;

		void main()
		{
			gl_Position = vPosition;
			uvDest = vUV * maxUVDest;
			uvSrc = vUV * maxUVSrc;
		}
	)";
	addParam("mixVal");
}

Mixer::~Mixer()
{
}

FFResult Mixer::ProcessOpenGL(ProcessOpenGLStruct * pGL)
{
	if (pGL->numInputTextures < 2) return FF_FAIL;
	if (pGL->inputTextures[0] == nullptr) return FF_FAIL;
	if (pGL->inputTextures[1] == nullptr) return FF_FAIL;

	//Activate our shader using the scoped binding so that we'll restore the context state when we're done.
	ScopedShaderBinding shaderBinding(shader.GetGLID());

	glUniform1i(shader.FindUniform("textureDest"), 0);
	glUniform1i(shader.FindUniform("textureSrc"), 1);
	//The input texture's dimension might change each frame and so might the content area.
	//We're adopting the texture's maxUV using a uniform because that way we dont have to update our vertex buffer each frame.
	FFGLTextureStruct& TextureDest = *pGL->inputTextures[0];
	FFGLTexCoords maxCoordsDest = GetMaxGLTexCoords(TextureDest);
	glUniform2f(shader.FindUniform("maxUVDest"), maxCoordsDest.s, maxCoordsDest.t);

	FFGLTextureStruct& TextureSrc = *pGL->inputTextures[1];
	FFGLTexCoords maxCoordsSrc = GetMaxGLTexCoords(TextureSrc);
	glUniform2f(shader.FindUniform("maxUVSrc"), maxCoordsSrc.s, maxCoordsSrc.t);

	//The shader's samplers are fixed so we need to bind the texture to these exact sampler indices. Use the scoped
	//bindings to ensure that the context will be returned in it's default state after we're done rendering.
	ScopedSamplerActivation activateSampler0(0);
	Scoped2DTextureBinding textureBinding0(TextureDest.Handle);
	ScopedSamplerActivation activateSampler1(1);
	Scoped2DTextureBinding textureBinding1(TextureSrc.Handle);

	return Plugin::ProcessOpenGL(pGL);
}