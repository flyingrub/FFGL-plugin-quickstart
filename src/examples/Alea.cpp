#include "Alea.h"

static PluginInstance p = Source::createPlugin<Alea>({
	"FL01", // plugin unique ID
	"ALEA" // Plugin name
});

static const std::string fshader = R"(
#define PI 3.14159265359
#define TWO_PI 6.28318530718
uniform float sides;

vec2 rotate2D(vec2 _uv, float _angle){
    _uv =  mat2(cos(_angle),-sin(_angle),
                sin(_angle),cos(_angle)) * _uv;
    return _uv;
}

float polygon(vec2 _uv, float size, float width, float sides) {
	// Angle and radius from the current pixel
	float a = atan(_uv.x,_uv.y)+PI;
	float r = TWO_PI/float(sides);

	// Shaping function that modulate the distance
	float d = cos(floor(.5+a/r)*r-a)*length(_uv);

	return 1.-smoothstep(size+width,size+width+0.005,d)-1.+smoothstep(size,size+0.005,d);
}

void main()
{
    vec2 uv = i_uv*2.-1.;
	uv.x *= resolution.x/resolution.y;
    uv = rotate2D(uv, time*0.1 +  audioVolume);

    float size = iSize/10. + iSize * audioVolume;
    float width = iSize/40. + iSize * audioVolume * .5;
    float rgbShift = iShiftAmount * audioVolume * .1;

	float colorR = polygon(uv-vec2(rgbShift,0), size, width, sides);
    float colorG = polygon(uv, size, width, sides);
    float colorB = polygon(uv+vec2(rgbShift,0), size, width, sides);
	vec3 color = vec3(colorR, colorG, colorB);
    fragColor = vec4(color,1.0);
}
)";

Alea::Alea()
{
	setFragmentShader(fshader);
	addHueColorParam("color");
	addParam("iSize", .6);
	addParam("iShiftAmount",0.5);
	addParam("smoothness", 0.9);
	addButtonParam("change");
	addBoolParam("fixedSize");
	Param optionParam = Param("select", FF_TYPE_OPTION, 1.0f);
	optionParam.options = {
		{"Global", 1.0f},
		{"Bass", 2.0f},
		{"Med", 3.0f},
		{"High", 4.0f },
	};
	addOptionParam(optionParam);
	addTextParam("test");
	addParam(Param("gainParam", 0.5, {-42,42}));
}

void Alea::update() {
	Param change = getParam("change");

	if (change.value == 1) {
		sides = random.getRandomInt(3, 10);
	}

	if (getParam("fixedSize").value) {
		glUniform1f(shader.FindUniform("audioVolume"), 1.0f);
	}

	Param select = getParam("select");
	if (select.value == 2.0f) {
		glUniform1f(shader.FindUniform("audioVolume"), audio.getBass());
	} else if(select.value == 3.0f) {
		glUniform1f(shader.FindUniform("audioVolume"), audio.getMed());
	} else if(select.value == 4.0f) {
		glUniform1f(shader.FindUniform("audioVolume"), audio.getHigh());
	}

	glUniform1f(shader.FindUniform("sides"), sides);

	Param smoothness = getParam("smoothness");
	audio.setSmoothness(smoothness.getValue());
	audio.gainParam = getParam("gainParam").getValue();
}

Alea::~Alea()
{
}
