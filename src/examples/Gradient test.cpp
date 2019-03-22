#include "Gradient test.h"

static PluginInstance p = Source::createPlugin<GradientTest>({
	"FL07", // plugin unique ID
	"Gradient test" // Plugin name
});

static const std::string fshader = R"(
void main()
{
	 vec2 uv = fragCoord/resolution.xy;
	uv.x *= resolution.x/resolution.y;
    
    // Time varying pixel color
    vec3 col = 0.5 + 0.5*cos(time+uv.xyx+vec3(0,2,4));

    vec2 bl = step(vec2(amount*0.1),uv);       // bottom-left
    vec2 tr = step(vec2(amount*0.1),vec2(1.*resolution.x/resolution.y,1.)-uv);   // top-right
    vec3 color = vec3(bl.x * bl.y * tr.x * tr.y);

    // Output to screen
    fragColor = vec4(col * 1.-color,1.0);
}
)";

GradientTest::GradientTest()
{
	setFragmentShader(fshader);
	addParam("amount");
}

GradientTest::~GradientTest()
{
}