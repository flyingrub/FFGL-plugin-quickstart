#include "Scanline.h"

static PluginInstance p = Effect::createPlugin<Scanline>({
	"FL06", // plugin unique ID
	"Scanline" // Plugin name
	});

static const std::string fshader = R"(
float random( vec2 fragCoord )
{
    uvec2 x = uvec2(fragCoord) + 1920U*1080U*uint(frame);
    uvec2 q = 1103515245U * ( (x>>1U) ^ (x.yx   ) );
    uint  n = 1103515245U * ( (q.x  ) ^ (q.y>>3U) );
    return float(n) * (1.0/float(0xffffffffU));
}
void main()
{
	vec4 color = texture(inputTexture, uv);
	vec2 fragCoord = uv * resolution.xy;
	//The inputTexture contains premultiplied colors, so we need to unpremultiply first to apply our effect on straight colors.
	if (color.a > 0.0)
		color.rgb /= color.a;

	float count = resolution.y * density;
	vec3 col = color.rgb;
	vec2 sl = vec2(sin(uv.y * count), cos(uv.y * count));
	vec3 scanlines = vec3(sl.x, sl.y, sl.x);

	col += col * scanlines * opacityScanline;
	col += col * vec3(random(fragCoord)) * opacityNoise;
	
	//The plugin has to output premultiplied colors, this is how we're premultiplying our straight color while also
	//ensuring we aren't going out of the LDR the video engine is working in.
	color.rgb = clamp(col * color.a, vec3(0.0), vec3(color.a));
	fragColor = color;
}
)";

Scanline::Scanline()
{
	setFragmentShader(fshader);
	addParam(Param("density", .5, {0.,2.}));
	addParam(Param("opacityScanline", .4, { 0.,2. }));
	addParam(Param("opacityNoise", .6, { 0.,4. }));
}
Scanline::~Scanline()
{
}
