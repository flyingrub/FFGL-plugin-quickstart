#include "Color.h"

static PluginInstance p = Source::createPlugin<Color>({
	"SX01", // plugin unique ID
	"ADDEX" // Plugin name
});

std::string fShaderMain = R"(
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
    vec2 uv = fragCoord*2.-1.;
	uv.x *= resolution.x/resolution.y;
    uv = rotate2D(uv, time*0.1 +  audioVolume *20.);
	
    float seed = 8.; // = floor(audioBass * 5.);
    float size = iSize/10. + iSize * audioVolume *60.;
    float width = iSize/40. + iSize * audioVolume *20.;
    float rgbShift = iShiftAmount * audioVolume *6.;
    float colorR = polygon(uv-vec2(rgbShift,0), size, width, sides);
    float colorG = polygon(uv, size, width, sides);
    float colorB = polygon(uv+vec2(rgbShift,0), size, width, sides);
	vec3 color = vec3(colorR, colorG, colorB);
    fragColor = vec4(color,1.0);
}
)";

Color::Color()
{
	setFragmentShader(fShaderMain);
	addHueColorParam("color");
	addParam("iSize", 1);
	addParam("iShiftAmount",0.5);
	addButtonParam("change");
	addBoolParam("placed");

}

void Color::update() {
	Param change = getParam("change");
	if (change.currentValue == 1) {
		sides = random.getRandomInt(3, 10);
	}
	glUniform1f(shader.FindUniform("sides"), sides);
}

Color::~Color()
{
}
