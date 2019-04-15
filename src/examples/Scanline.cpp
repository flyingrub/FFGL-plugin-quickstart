#include "Scanline.h"

static PluginInstance p = Effect::createPlugin< Scanline >( {
	"FL06",   // plugin unique ID
	"Scanline"// Plugin name
} );

static const std::string fshader = R"(
void main()
{
	vec4 color = texture(inputTexture, i_uv);
	//The inputTexture contains premultiplied colors, so we need to unpremultiply first to apply our effect on straight colors.
	if (color.a > 0.0)
		color.rgb /= color.a;

	float count = resolution.y * density;
	vec3 col = color.rgb;
	vec2 sl = vec2(sin(i_uv.y * count), cos(i_uv.y * count));
	vec3 scanlines = vec3(sl.x, sl.y, sl.x);

	col += col * scanlines * opacityScanline;
	col += col * vec3(random()) * opacityNoise;
    col += col * sin(110.0*time) * flickering;

	
	//The plugin has to output premultiplied colors, this is how we're premultiplying our straight color while also
	//ensuring we aren't going out of the LDR the video engine is working in.
	color.rgb = clamp(col * color.a, vec3(0.0), vec3(color.a));
	fragColor = color;
}
)";

Scanline::Scanline()
{
	include( shader::snippet_id::random );
	setFragmentShader( fshader );
	addParam( ParamRange::create( "density", .5, { 0., 2. } ) );
	addParam( ParamRange::create( "opacityScanline", .4, { 0., 2. } ) );
	addParam( ParamRange::create( "opacityNoise", .6, { 0., 4. } ) );
	addParam( ParamRange::create( "flickering", .3, { 0., .1 } ) );
}
Scanline::~Scanline()
{
}
