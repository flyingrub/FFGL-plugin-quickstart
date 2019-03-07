#include "Color.h"

static CFFGLPluginInfo PluginInfo(
	PluginFactory< Color >,			// Create method
	"SX01",							// Plugin unique ID
	"Color",						// Plugin name
	2,								// API major version number
	1,								// API minor version number
	1,								// Plugin major version number
	000,							// Plugin minor version number
	FF_SOURCE,						// Plugin type
	"Sample FFGL Source plugin",	// Plugin description
	"Resolume FFGL Example"			// About
);

std::string fShader = R"(
#version 410 core
in vec2 uv;
uniform vec3 color;
out vec4 fragColor;

void main()
{
	fragColor = vec4(color, 1.0);
}
)";

Color::Color()
{
	setFragmentShader(fShader);
	addColorParam("color");
}

Color::~Color()
{
}
