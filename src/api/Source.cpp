#include "Source.h"

Source::Source()
{
	SetMinInputs(0);
	SetMaxInputs(0);
	fragmentShaderCodeStart += R"(
		in vec2 fragCoord;
	)";
	vertexShaderCode = R"(
		#version 410 core
		layout( location = 0 ) in vec4 vPosition;
		layout( location = 1 ) in vec2 vUV;

		out vec2 fragCoord;

		void main()
		{
			gl_Position = vPosition;
			fragCoord = vUV;
		}
	)";
}

Source::~Source()
{
}