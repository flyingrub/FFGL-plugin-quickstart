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
	float t = time;
	vec2 r = resolution;
	vec3 c;
	float l;
	float z=t;
	for(int i=0;i<3;i++) {
		vec2 m_uv = uv;
		vec2 p=m_uv;
		p-=.5;
		p.x*=r.x/r.y;
		z+=.07;
		l=length(p);
		m_uv+=p/l*(sin(z)+1.)*abs(sin(l*9.-z*2.));
		c[i]=.01/length(abs(mod(m_uv,1.)-.5));
	}
	fragColor=vec4(c/l,t);
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
