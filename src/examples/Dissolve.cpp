#include "Dissolve.h"

static PluginInstance p = Effect::createPlugin<Dissolve>({
	"FL09", // plugin unique ID
	"Dissolve" // Plugin name
});

static const std::string fshader = R"(
#define pixel_width 3./resolution.y

float fractal_noise(vec3 m) {
    float start = 0.5;
	vec4 coef = vec4(start,start/2.,start/4.,start/8.);
	coef.yzw *= 1.-smoothness;
    return fractal_noise(m,coef);
}

void main()
{
	vec2 p = i_uv;
	vec3 p3 = vec3(p*zoom, time*speed)*8.0+8.0;
    
	float red = fractal_noise(p3)*.5+.5;
	red = smoothstep(pixel_width,0.,red-amount);
    
    p3 = p3+spaced;
    float blue = fractal_noise(p3)*.5+.5;
    blue = smoothstep(pixel_width,0.,blue-amount);
   
    p3 = p3+spaced;
    float green = fractal_noise(p3)*.5+.5;
	green = smoothstep(pixel_width,0.,green-amount);

    vec3 color = texture(inputTexture,i_uv).rgb;
	vec3 col = vec3(red,green,blue)*color;
	
	fragColor = vec4(vec3(col),1.0);
}
)";

Dissolve::Dissolve()
{
	include(shader::snippet_id::simplex);
	addParam(ParamRange::create("zoom", 0.5f, {0.0, 2.0}));
	addParam(ParamRange::create("speed", 0.5f, { 0.0, 0.2 }));
	addParam(ParamRange::create("amount", 0.5f, { 0.0, 1.0 }));
	addParam(ParamRange::create("spaced", 0.5f, { 0.0, .01 }));
	addParam(ParamRange::create("smoothness", 0.5f, { 0.0, 1.0 }));
	setFragmentShader(fshader);
}

Dissolve::~Dissolve()
{
}