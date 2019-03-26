#include "Polygon.h"

static PluginInstance p = Source::createPlugin<PolygonRepeat>({
	"FL05", // plugin unique ID
	"Polygon" // Plugin name
});

static const std::string fshader = R"(
precision highp float;
#define PI 3.14159265359
#define TWO_PI 6.28318530718
#define pixel_width 50.*repeat*3./resolution.y

float stroke(float d, float size) {
	return smoothstep(pixel_width,0.0,abs(d-size)-width/2.);
}

vec2 rotate(vec2 _uv, float _angle){
    _uv =  mat2(cos(_angle),-sin(_angle),
                sin(_angle),cos(_angle)) * _uv;
    return _uv;
}

float polygonSDF(vec2 _uv) {
	// Angle and radius from the current pixel
	float a = atan(_uv.x,_uv.y)+PI;
	float r = TWO_PI/float(floor(sides));

	return cos(floor(.5+a/r)*r-a)*length(_uv);
}

float smoothmodulo(float a) {
	return abs( mod(a, 2.) - 1.);
}

void main()
{
    vec2 U = i_uv*2.-1.;
	U.x *= resolution.x/resolution.y;
    U = rotate(U,time*rotation_speed);
    U *= repeat*50.;
    float c = stroke(smoothmodulo(polygonSDF(U)+time*10.*speed),1.);
    fragColor = vec4(vec3(c),1.);                                       
}
)";

PolygonRepeat::PolygonRepeat()
{
	setFragmentShader(fshader);
	addParam(Param("repeat", 0.05, { 0., 1. }));
	addParam(Param("speed", 0.4, { -1., 1. }));
	addParam(Param("sides", .3, { 1., 10. }));
	addParam(Param("width", 0.5, { 0., 2. }));
	addParam(Param("rotation_speed", 0.5, { -1., 1. }));
}

void PolygonRepeat::update()
{
}

PolygonRepeat::~PolygonRepeat()
{

}