#include "Mixer.h"



Mixer::Mixer()
{
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
	vertexShaderCode = R"(
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
}


Mixer::~Mixer()
{
}
