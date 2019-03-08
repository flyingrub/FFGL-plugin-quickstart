#include "Color.h"

static CFFGLPluginInfo infos(
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
void main()
{
	vec3 color = vec3(uv.x,uv.y,abs(sin(time))) * u_color;
	fragColor = vec4(color, 1.0);
}
)";

Color::Color()
{
	setFragmentShader(fShader);
	addColorParam("u_color");
}

Color::~Color()
{
}
