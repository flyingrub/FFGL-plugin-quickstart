#include "Strobotime.h"

static PluginInstance p = Effect::createPlugin<Strobotime>({
	"FL12", // plugin unique ID
	"Strobotime" // Plugin name
});

static const std::string fshader = R"(
void main()
{
	vec4 tex = texture(inputTexture, i_uv);
	bool shouldBeOn = fract(phase * speed) < dutyCycle;
	
	fragColor=mix(tex,vec4(color,1.),shouldBeOn);
}
)";

Strobotime::Strobotime()
{
	addHueColorParam("color");
	addParam(dutyCycle = Param::create("dutyCycle"));
	addParam(speed = ParamRange::create("speed", .5, { 1 , 64 }));
	setFragmentShader(fshader);
}

void Strobotime::update() {
	unsigned long v = speed->getValueNormalised();
	v--; // 
	v |= v >> 1;
	v |= v >> 2;
	v |= v >> 4;
	v |= v >> 8;
	v |= v >> 16;
	v++;
	float value = (float)v / 16.f;
	glUniform1f(shader.FindUniform("speed"), value);
}

Strobotime::~Strobotime()
{
}