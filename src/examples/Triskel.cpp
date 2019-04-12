#include "Triskel.h"

static PluginInstance p = Source::createPlugin<Triskel>({
	"FL11", // plugin unique ID
	"Triskel" // Plugin name
});

static const std::string fshader = R"(
void main()
{
	vec2 R = resolution.xy;
	vec2 U = i_uv*R;
	float iTime = time;
    U = (U+U-R)/R.y + vec2(0,.1);
    
    float tau = 6.283,                                       // 3 symmetries
            a = -floor((atan(U.y,U.x)-.33)*3./tau)/3.*tau -.05, l;
    U *= mat2(cos(a),-sin(a),sin(a),cos(a));
    U = 3.*(U-(vec2(0,.350)+vec2(0,.350)*cos(iTime*.03)));
    

    l = length(U), a = atan(U.y,U.x);                        // spiral
	vec4 O = vec4( l + fract((a+2.25)/tau) < 5. + cos(iTime*.2) * 2. ? 0.5+.5*sin(a+tau*l*sin(iTime*.2)*5.) : 0.);
    
	O = smoothstep(.1*cos(iTime*.03),.0,abs(O-.5));   // optional decoration
	fragColor = O;
}
)";

Triskel::Triskel()
{
	setFragmentShader(fshader);
}

Triskel::~Triskel()
{
}