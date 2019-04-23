#include "Plugin.h"
using namespace ffglex;

static const int FFT_INPUT_INDEX = 0;

Plugin::Plugin()
{
	SetBufferParamInfo( FFT_INPUT_INDEX, "FFT", Audio::getBufferSize(), FF_USAGE_FFT );
}

Plugin::~Plugin()
{
}

FFResult Plugin::InitGL( const FFGLViewportStruct* viewPort )
{
	std::string fragmentShaderCode = fragmentShaderCodeStart;
	int i                          = 0;
	while( i < params.size() )
	{
		if( isRGBColor( i ) || isHueColor( i ) )
		{
			fragmentShaderCode += "uniform vec3 " + params[ i ]->getName() + ";\n";
			i += 2;
		}
		else if( params[ i ]->getType() == FF_TYPE_BOOLEAN || params[ i ]->getType() == FF_TYPE_EVENT )
		{
			fragmentShaderCode += "uniform bool " + params[ i ]->getName() + ";\n";
		}
		else
		{
			fragmentShaderCode += "uniform float " + params[ i ]->getName() + ";\n";
		}
		i += 1;
	}

	for( auto snippet : includedSnippets )
	{
		fragmentShaderCode += shader::snippets.find( snippet )->second;
	}

	fragmentShaderCode += fragmentShader;
	if( !shader.Compile( vertexShaderCode, fragmentShaderCode ) )
	{
		DeInitGL();
		return FF_FAIL;
	}
	if( !quad.Initialise() )
	{
		DeInitGL();
		return FF_FAIL;
	}
	init();

	return CFreeFrameGLPlugin::InitGL( viewPort );
}

FFResult Plugin::ProcessOpenGL( ProcessOpenGLStruct* inputTextures )
{
	ScopedShaderBinding shaderBinding( shader.GetGLID() );
	// Clamp to edge is broken in Resolume right now so disable it
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER );

	int i = 0;
	while( i < params.size() )
	{
		if( isRGBColor( i ) )
		{
			std::string name = params[ i ]->getName();
			float r          = params[ i ]->getValue();
			float g          = params[ i + 1 ]->getValue();
			float b          = params[ i + 2 ]->getValue();
			glUniform3f( shader.FindUniform( name.c_str() ), r, g, b );
			i += 2;
		}
		else if( isHueColor( i ) )
		{
			float rgb[ 3 ];
			std::string name = params[ i ]->getName();
			float hue        = params[ i ]->getValue();
			float saturation = params[ i + 1 ]->getValue();
			float brightness = params[ i + 2 ]->getValue();
			//we need to make sure the hue doesn't reach 1.0f, otherwise the result will be pink and not red how it should be
			hue = ( hue == 1.0f ) ? 0.0f : hue;
			HSVtoRGB( hue, saturation, brightness, rgb[ 0 ], rgb[ 1 ], rgb[ 2 ] );
			glUniform3f( shader.FindUniform( name.c_str() ), rgb[ 0 ], rgb[ 1 ], rgb[ 2 ] );
			i += 2;
		}
		if( params[ i ]->getType() == FF_TYPE_BOOLEAN || params[ i ]->getType() == FF_TYPE_EVENT )
		{
			std::string name = params[ i ]->getName();
			glUniform1i( shader.FindUniform( name.c_str() ), (bool) params[ i ]->getValue() );
		}
		else
		{
			auto range       = std::dynamic_pointer_cast< ParamRange >( params[ i ] );
			std::string name = params[ i ]->getName();
			float value      = range ? range->getValueNormalised() : params[ i ]->getValue();
			glUniform1f( shader.FindUniform( name.c_str() ), value );
		}
		i += 1;
	}
	frame++;
	auto t_now      = std::chrono::high_resolution_clock::now();
	float timeNow   = std::chrono::duration< float, std::milli >( t_now - t_start ).count() / 1000.0f;
	deltaTime = timeNow - lastUpdate;
	lastUpdate      = timeNow;

	glUniform1f( shader.FindUniform( "time" ), timeNow );
	glUniform1f( shader.FindUniform( "deltaTime" ), deltaTime );
	glUniform1i( shader.FindUniform( "frame" ), frame );
	glUniform2f( shader.FindUniform( "resolution" ), (float) currentViewport.width, (float) currentViewport.height );

	std::vector< float > fftData( Audio::getBufferSize() );
	const ParamInfo* fftInfo = FindParamInfo( FFT_INPUT_INDEX );
	for( size_t index = 0; index < Audio::getBufferSize(); ++index )
		fftData[ index ] = fftInfo->elements[ index ].value;
	audio.update( fftData );
	glUniform1f( shader.FindUniform( "audioVolume" ), audio.getVolume() );
	glUniform1f( shader.FindUniform( "audioBass" ), audio.getBass() );
	glUniform1f( shader.FindUniform( "audioMed" ), audio.getMed() );
	glUniform1f( shader.FindUniform( "audioHigh" ), audio.getHigh() );

	glUniform1f( shader.FindUniform( "bpm" ), bpm );
	glUniform1f( shader.FindUniform( "phase" ), barPhase );

	update();
	quad.Draw();

	return FF_SUCCESS;
}

FFResult Plugin::DeInitGL()
{
	clean();
	shader.FreeGLResources();
	quad.Release();
	return FF_SUCCESS;
}

char* Plugin::GetParameterDisplay( unsigned int index )
{
	if( 0 < index && index <= params.size() )
	{
		if( params[ index - 1 ]->getType() == FF_TYPE_TEXT )
			return (char*)FF_FAIL;
		static char displayValueBuffer[ 16 ];
		auto range              = std::dynamic_pointer_cast< ParamRange >( params[ index - 1 ] );
		float value             = range ? range->getValueNormalised() : params[ index - 1 ]->getValue();
		std::string stringValue = std::to_string( value );
		memset( displayValueBuffer, 0, sizeof( displayValueBuffer ) );
		memcpy( displayValueBuffer, stringValue.c_str(), std::min( sizeof( displayValueBuffer ), stringValue.length() ) );
		return displayValueBuffer;
	}
	else
	{
		return CFreeFrameGLPlugin::GetParameterDisplay( index );
	}
}

FFResult Plugin::SetFloatParameter( unsigned int index, float value )
{
	if( index == FFT_INPUT_INDEX )
		return FF_SUCCESS;
	if( index <= params.size() )
	{
		params[ index - 1 ]->setValue( value );
		return FF_SUCCESS;
	}
	else
	{
		return FF_FAIL;
	}
}

float Plugin::GetFloatParameter( unsigned int index )
{
	if( 0 < index && index <= params.size() )
	{
		return params[ index - 1 ]->getValue();
	}
	else
	{
		return 0.0f;
	}
}

FFResult Plugin::SetTextParameter( unsigned int index, const char* value )
{
	bool inBounds = 0 < index && index <= params.size();
	if( !inBounds )
		return FF_FAIL;

	auto paramText = std::dynamic_pointer_cast< ParamText >( params[ index - 1 ] );
	if( !paramText )
		return FF_FAIL;

	paramText->text = value == nullptr ? "" : value;
	return FF_SUCCESS;
}

char* Plugin::GetTextParameter( unsigned int index )
{
	bool inBounds = 0 < index && index <= params.size();
	if( !inBounds )
		return "";

	auto paramText = std::dynamic_pointer_cast< ParamText >( params[ index - 1 ] );
	if( !paramText )
		return "";

	return const_cast< char* >( paramText->text.c_str() );
}

void Plugin::SetSampleRate( unsigned int _sampleRate )
{
	sampleRate = _sampleRate;
	audio.setSampleRate( sampleRate );
}

void Plugin::setFragmentShader( std::string fShader )
{
	fragmentShader = fShader;
}

void Plugin::addParam( Param::Ptr param )
{
	params.push_back( param );
	SetParamInfof( (unsigned int)params.size(), param->getName().c_str(), param->getType() );
}

void Plugin::addParam( ParamOption::Ptr param )
{
	params.push_back( param );
	unsigned int index = (unsigned int)params.size();
	SetOptionParamInfo( index, param->getName().c_str(), (unsigned int)param->options.size(), param->getValue() );

	for( unsigned int i = 0; i < param->options.size(); i++ )
	{
		SetParamElementInfo( index, i, param->options[ i ].c_str(), (float)i );
	}
}

void Plugin::addHueColorParam( std::string name )
{
	addParam( Param::create( name, FF_TYPE_HUE, 0. ) );
	addParam( Param::create( name + "_saturation", FF_TYPE_SATURATION, 0. ) );
	addParam( Param::create( name + "_brighthness", FF_TYPE_BRIGHTNESS, 1.0 ) );
}

void Plugin::addRGBColorParam( std::string name )
{
	addParam( Param::create( name, FF_TYPE_RED, 0.5 ) );
	addParam( Param::create( name + "_green", FF_TYPE_GREEN, 0.5 ) );
	addParam( Param::create( name + "_blue", FF_TYPE_BLUE, 0.5 ) );
}

bool Plugin::isHueColor( int index )
{
	bool enoughSpace = index + 2 < params.size();
	if( !enoughSpace )
		return false;
	bool isColorType =
		params[ index ]->getType() == FF_TYPE_HUE &&
		params[ index + 1 ]->getType() == FF_TYPE_SATURATION &&
		params[ index + 2 ]->getType() == FF_TYPE_BRIGHTNESS;

	return isColorType;
}

bool Plugin::isRGBColor( int index )
{
	bool enoughSpace = index + 2 < params.size();
	if( !enoughSpace )
		return false;
	bool isColorType =
		params[ index ]->getType() == FF_TYPE_RED &&
		params[ index + 1 ]->getType() == FF_TYPE_GREEN &&
		params[ index + 2 ]->getType() == FF_TYPE_BLUE;

	return isColorType;
}

Param::Ptr Plugin::getParam( std::string name )
{
	for( int i = 0; i < params.size(); i++ )
	{
		if( params[ i ]->getName().compare( name ) == 0 )
			return params[ i ];
	}
	return {};
}

ParamOption::Ptr Plugin::getParamOption( std::string name )
{
	auto param = getParam( name );
	if( !param )
		return {};
	auto option = std::dynamic_pointer_cast< ParamOption >( param );
	if( !option )
		return {};
	return option;
}

ParamText::Ptr Plugin::getParamText( std::string name )
{
	auto param = getParam( name );
	if( !param )
		return {};
	auto text = std::dynamic_pointer_cast< ParamText >( param );
	if( !text )
		return {};
	return text;
}

void Plugin::include( shader::snippet_id snippet )
{
	if( includedSnippets.find( snippet ) != includedSnippets.end() )
		return;

	includedSnippets.insert( snippet );
	auto deps = shader::dependencies.find( snippet );
	if( deps == shader::dependencies.end() )
		return;
	for( auto dep : deps->second )
	{
		includedSnippets.insert( dep );
	}
}

void Plugin::include( std::set< shader::snippet_id > snippets )
{
	for( auto snippet : snippets )
	{
		include( snippet );
	}
}
