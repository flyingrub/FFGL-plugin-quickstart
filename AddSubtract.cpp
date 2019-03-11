#include "AddSubtract.h"

static CFFGLPluginInfo infos(
	PluginFactory< AddSubtract >,			// Create method
	"SX01",							// Plugin unique ID
	"AddSubstract EX",						// Plugin name
	2,								// API major version number
	1,								// API minor version number
	1,								// Plugin major version number
	000,							// Plugin minor version number
	FF_EFFECT,						// Plugin type
	"Sample FFGL Source plugin",	// Plugin description
	"Resolume FFGL Example"			// About
);

std::string efShaderMain = R"(
void main()
{
	vec4 color = texture(inputTexture, uv);
	//The inputTexture contains premultiplied colors, so we need to unpremultiply first to apply our effect on straight colors.
	if (color.a > 0.0)
		color.rgb /= color.a;
	vec3 add = -1.0 + brightness * 2.0;
	color.rgb = color.rgb + add;

	//The plugin has to output premultiplied colors, this is how we're premultiplying our straight color while also
	//ensuring we aren't going out of the LDR the video engine is working in.
	color.rgb = clamp(color.rgb * color.a, vec3(0.0), vec3(color.a));
	fragColor = color;
}
)";

AddSubtract::AddSubtract()
{
	setFragmentShader(efShaderMain);
	addRGBColorParam("brightness");
}


AddSubtract::~AddSubtract()
{
}
