#include "Source.h"
using namespace ffglex;

static CFFGLPluginInfo PluginInfo(
	PluginFactory< Source >,		// Create method
	"EX01",							// Plugin unique ID
	"Source Example",				// Plugin name
	2,								// API major version number
	1,								// API minor version number
	1,								// Plugin major version number
	000,							// Plugin minor version number
	FF_SOURCE,						// Plugin type
	"Sample FFGL Source plugin",	// Plugin description
	"Resolume FFGL Example"			// About
);
static const char vertexShaderCode[] = R"(#version 410 core
layout( location = 0 ) in vec4 vPosition;
layout( location = 1 ) in vec2 vUV;

out vec2 uv;

void main()
{
	gl_Position = vPosition;
	uv = vUV;
}
)";

static const char fragmentShaderCode[] = R"(#version 410 core
uniform vec3 RGBLeft;
uniform vec3 RGBRight;

in vec2 uv;

out vec4 fragColor;

void main()
{
	fragColor = vec4(1.0, 0.0, 0.0, 1.0 );
}
)";
Source::Source()
{
}

Source::~Source()
{
}

FFResult Source::InitGL(const FFGLViewportStruct * vp)
{
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
	quad.Draw();

	return FF_SUCCESS;
}

FFResult Source::DeInitGL()
{
	shader.FreeGLResources();
	quad.Release();
	return FF_SUCCESS;
}

FFResult Source::SetFloatParameter(unsigned int dwIndex, float value)
{
	return FF_SUCCESS;
}

float Source::GetFloatParameter(unsigned int index)
{
	return 0.0f;
}
