#include "Bloom.h"
#include <cmath>

using namespace ffglex;

static PluginInstance p = Effect::createPlugin< Bloom >( {
	"FL13",// plugin unique ID
	"Bloom"// Plugin name
} );

static const std::string thresholdShader = R"(
void main()
{
	vec4 c = texture( inputTexture, i_uv );
	float brightness = max(max(c.r, c.g), c.b);
	c.rgb = mix(vec3(0), c.rgb, step(threshold, brightness));
	fragColor = vec4(c);
}
)";

// Downsample with a 4x4 box filter
static const std::string downsampleFilterShader = R"(
#version 410 core
out vec4 fragColor;

in vec2 i_uv;
uniform vec2 texelSize;

uniform sampler2D inputTexture;
void main()
{
	vec4 d = texelSize.xyxy * vec4(-1, -1, +1, +1);
	vec4 col = texture( inputTexture, i_uv + d.xy );
	col += texture( inputTexture, i_uv + d.zy );
	col += texture( inputTexture, i_uv + d.xw );
	col += texture( inputTexture, i_uv + d.zw );
	col /= 4.;
	fragColor = vec4(col);
}
)";

// 4-tap bilinear upsampler
static const std::string upsampleFilterShader = R"(
#version 410 core
out vec4 fragColor;

uniform sampler2D inputTexture;
uniform sampler2D baseTexture;

in vec2 i_uv;
uniform float sampleScale;
uniform vec2 texelSize;

void main()
{
	vec4 base = texture( baseTexture, i_uv );
	vec4 d = texelSize.xyxy * vec4(-1, -1, +1, +1) * sampleScale * 0.5;
	vec4 col = texture( inputTexture, i_uv + d.xy );
	col += texture( inputTexture, i_uv + d.zy );
	col += texture( inputTexture, i_uv + d.xw );
	col += texture( inputTexture, i_uv + d.zw );
	col /= 4.;
	fragColor = vec4(base.rgb + col.rgb,base.a);
}
)";

static const std::string finalShader = R"(
#version 410 core
out vec4 fragColor;

uniform sampler2D inputTexture;
uniform sampler2D baseTexture;

in vec2 i_uv;
uniform float intensity;
uniform float sampleScale;
uniform vec2 texelSize;

void main()
{
	vec4 base = texture( baseTexture, i_uv );
	vec4 d = texelSize.xyxy * vec4(-1, -1, +1, +1) * sampleScale * 0.5;
	vec4 col = texture( inputTexture, i_uv + d.xy );
	col += texture( inputTexture, i_uv + d.zy );
	col += texture( inputTexture, i_uv + d.xw );
	col += texture( inputTexture, i_uv + d.zw );
	col /= 4.;
	vec3 outColor = base.rgb + col.rgb * intensity;
	fragColor = vec4( col.rgb * intensity, base.a);
}
)";

Bloom::Bloom()
{
	setFragmentShader( thresholdShader );
	addParam( threshold = ParamRange::create( "threshold", 0.8f, { 0, 1 } ) );
	addParam( radius = ParamRange::create( "radius", 2.5f, { 1, 7 } ) );
	addParam( intensity = ParamRange::create( "intensity", 0.8f, { 0, 1 } ) );
}

FFResult Bloom::init()
{
	if( !downSampleFilter.Compile( vertexShaderCode, downsampleFilterShader ) )
	{
		DeInitGL();
		return FF_FAIL;
	}
	if( !upSampleFilter.Compile( vertexShaderCode, upsampleFilterShader ) )
	{
		DeInitGL();
		return FF_FAIL;
	}
	if( !final.Compile( vertexShaderCode, finalShader ) )
	{
		DeInitGL();
		return FF_FAIL;
	}
	return FF_SUCCESS;
}

FFResult Bloom::render( ProcessOpenGLStruct* inputTextures )
{
	static const int kMaxIterations = 16;

	// determine the iteration count
	double logh        = std::log( currentViewport.height ) / std::log( 2 ) + radius->getRealValue() - 8;
	int logh_i        = (int)logh;
	int iterations    = std::clamp( logh_i, 1, kMaxIterations );
	double sampleScale = 0.5f + logh - logh_i;

	FFGLFBO thresholdFBO;
	FFGLFBO mipmaps[ kMaxIterations ];
	FFGLFBO combine[ kMaxIterations ];

	// Prefilter pixel above a certain brightness threshold
	thresholdFBO.Create( currentViewport.width, currentViewport.height );
	thresholdFBO.BindAsRenderTarget();
	FFResult result = Effect::render( inputTextures );
	if( result == FF_FAIL )
		return FF_FAIL;

	FFGLFBO* last = &thresholdFBO;
	// Create a mipmap pyramid
	downSampleFilter.Use();
	downSampleFilter.Set( "maxUV", 1.f, 1.f );
	for( int i = 0; i < iterations; i++ )
	{
		downSampleFilter.Bind( "inputTexture", 0, last->GetTextureInfo() );
		mipmaps[ i ].Create( last->GetWidth() / 2, last->GetHeight() / 2 );
		mipmaps[ i ].BindAsRenderTarget();
		downSampleFilter.Set( "texelSize", 1.0f / (float)last->GetWidth(), 1.0f / (float)last->GetHeight() );
		quad.Draw();
		last = &mipmaps[ i ];
	}

	// upsample and combine loop
	upSampleFilter.Use();
	upSampleFilter.Set( "maxUV", 1.f, 1.f );
	for( int i = iterations - 2; i >= 0; i-- )
	{
		upSampleFilter.Bind( "inputTexture", 0, last->GetTextureInfo() );
		upSampleFilter.Bind( "baseTexture", 1, mipmaps[ i ].GetTextureInfo() );
		combine[ i ].Create( mipmaps[ i ].GetWidth(), mipmaps[ i ].GetHeight());
		combine[ i ].BindAsRenderTarget();
		upSampleFilter.Set( "texelSize", 1.0f / (float)last->GetWidth(), 1.0f / (float)last->GetHeight() );
		upSampleFilter.Set( "sampleScale", (float)sampleScale );
		quad.Draw();
		last = &combine[ i ];
	}

	glBindFramebuffer( GL_FRAMEBUFFER, inputTextures->HostFBO );
	final.Use();
	final.Set( "maxUV", 1.f, 1.f );
	final.Bind( "inputTexture", 0, last->GetTextureInfo() );
	final.Bind( "baseTexture", 1, *inputTextures->inputTextures[ 0 ] );
	final.Set( "intensity", intensity->getRealValue() );
	final.Set( "texelSize", 1.0f / (float)last->GetWidth(), 1.0f / (float)last->GetHeight() );
	final.Set( "sampleScale", (float)sampleScale );
	quad.Draw();

	// Free all allocated ressource
	for( int i = 0; i < iterations; i++ )
	{
		mipmaps[ i ].FreeResources();
		combine[ i ].FreeResources();
	}
	thresholdFBO.FreeResources();
	return FF_SUCCESS;
}

void Bloom::clean()
{
	downSampleFilter.FreeGLResources();
	upSampleFilter.FreeGLResources();
	final.FreeGLResources();
}

Bloom::~Bloom()
{
}