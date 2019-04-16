#pragma once
#include <FFGLSDK.h>
#include <vector>
#include <string>
#include "../helpers/Audio.h"
#include "../helpers/Utils.h"
#include <chrono>
#include "Params.h"

typedef CFFGLPluginInfo PluginInstance;

struct PluginInfo
{
	std::string id, name, description, about;
	int majorVersion = 1;
	int minorVersion = 0;
};

class Plugin : public CFreeFrameGLPlugin
{
public:
	Plugin();
	~Plugin();

	/// This method allows to actually create an FFGL Plugin and will take care of
	/// instanciancing and all of the boring stuff
	template< typename PluginType >
	static PluginInstance createPlugin( PluginInfo infos, FFUInt32 type );

	/// Implementation of the FFGL InitGL instance specific function. This function allocates
	/// the OpenGL resources the plugin needs during its lifetime. In this function we assemble the
	/// different bit of the fragment shader into one and try to compile it. If the shader fails to
	/// compile we return FF_FAIL.
	///
	/// \param	viewPort		Pointer to a FFGLViewportStruct structure (see the definition in FFGL.h
	///							and the description in the FFGL specification).
	/// \return					This implementation returns FF_SUCCESS if the shader did compile
	///							correctly FF_FAIL otherwise.
	FFResult InitGL( const FFGLViewportStruct* viewPort ) override;
	/// Implementation of the FFGL ProcessOpenGL instance specific function. It is in this function that
	/// the actual rendering occur. This function takes care of sending all the parameter to the shader.
	/// 
	/// \param	inputTextures	This is a parameter containing info about input texture if there is some.
	/// \return					This implementation always returns FF_SUCCESS.
	FFResult ProcessOpenGL( ProcessOpenGLStruct* inputTextures ) override;
	/// Implementation of the FFGL DeInitGL instance specific function. This function frees
	/// any OpenGL resources the plugin has allocated
	///
	/// \return					This implementation always returns FF_SUCCESS.
	FFResult DeInitGL() override;
	
	/// This function is called by InitGL just after the main shader managed to compile.
	virtual void init(){};
	/// This function is called by ProcessOpenGL just before the main shader draw call.
	virtual void update(){};
	/// This function is called by DeInitGL before freeing any ressources.
	virtual void clean(){};

	/// This function is called by the host to get a string representation of any parameter.
	/// It will just return the value of the current param as a char*.
	/// \param	index		The index of the parameter to display
	/// \return				The char* representation of the parameter
	char* GetParameterDisplay( unsigned int index ) override;
	/// This function is called by the host to set a value to a specific param thanks to its index.
	/// \param	index		The index of the parameter to set
	/// \param	value		The value to assign to the param
	/// \return				FF_SUCCESS if we managed to find the parameter and set its value
	FFResult SetFloatParameter( unsigned int index, float value ) override;
	/// This function is called by the host to get the float value of a Param thanks to its index.
	/// \param	index		The index of the parameter to get
	/// \return				The float value of the parameter
	float GetFloatParameter( unsigned int index ) override;
	/// This function is called by the host to set the text value of a ParamText.
	/// \param	index		The index of the parameter to set
	/// \param	value		The text to assign to the param
	/// \return				FF_SUCCESS if we managed to find the parameter and set its value
	FFResult SetTextParameter( unsigned int index, const char* value ) override;
	/// This function is called by the host to get the text value of a ParamText.
	/// \param	index		The index of the parameter to get
	/// \return				The text assigned to the parameter
	char* GetTextParameter( unsigned int index ) override;
	/// This function is called by the host when the current sample rate has changed.
	/// \param	sampleRate	The new sample rate
	void SetSampleRate( unsigned int sampleRate ) override;

	/// This function must be called in the constructor of your plugin. It will allows to
	/// construct the full fragment shader. In order to simplify the development of plugin
	/// it is InitGL that takes care of defining all the parameters so that the shader can acces
	/// them.
	/// \param	fShader		The fragment shader
	void setFragmentShader( std::string fShader );
	/// This function allows you to add a new parameter to the plugin. There is differents kind of
	/// parameters available, you can check them in Params.h. Adding a parameter allows the plugin to be
	/// aware of them, make them available to the host and take of all the communication with it.
	/// It also allows the plugin to automatically pass the current value of each parameters to the
	/// shader before drawing.
	/// \param	param		The parameter to add
	void addParam( Param::Ptr param );
	/// This function handle the special case where the parameter is a ParamOption (When you habe the
	/// choice between different option).
	/// \param	param		The parameter to add
	void addParam( ParamOption::Ptr param );
	/// This function allows to create a Hue color param, for exemple in Resolume this will display a
	/// color picker, which is very handy to choose your color.
	/// \param	name		The name of the parameter to add
	void addHueColorParam( std::string name );
	/// This function allows to create a RGB color param, in Resolume it is displayed as three sliders for
	/// each Red, Green and Blue.
	/// \param	name		The name of the parameter to add
	void addRGBColorParam( std::string name );
	/// This function allows to check if a certains index corresponds to a Hue color. Internally, it is
	/// represented as three Params. It checks if the current one and the two followings are actually
	/// corresponding to a Hue color.
	/// \param	index		The index of the first parameter that could correspond to a Hue color.
	bool isHueColor( int index );
	/// This function allows to check if a certains index corresponds to a RGB color. Internally, it is
	/// represented as three Params. It checks if the current one and the two followings are actually
	/// corresponding to a RGB color.
	/// \param	index		The index of the first parameter that could correspond to a RGB color.
	bool isRGBColor( int index );

	/// This function allows to get a parameter by it's name.
	/// \param	name		The name of the parameter to get.
	/// \return				The parameter if we managed to find it.
	Param::Ptr getParam( std::string name );
	/// This function allows to get an option parameter by it's name.
	/// \param	name		The name of the parameter to get.
	/// \return				The parameter if we managed to find it.
	ParamOption::Ptr getParamOption( std::string name );
	/// This function allows to get a text parameter by it's name.
	/// \param	name		The name of the parameter to get.
	/// \return				The parameter if we managed to find it.
	ParamText::Ptr getParamText( std::string name );

	/// This function allows to include snippet of code that are provided in Utils.h. This allows plugin
	/// programmer to not rewrite common code in each plugin. For example, code to get a random number,
	/// map a value from a range to another, and simplex noise are currently available.
	/// \param	snippet		The id of a snippet to include.
	void include( shader::snippet_id snippet );
	/// This function will call the function just above for each snippets in the set.
	/// \param	snippets		A set of snippets to include.
	void include( std::set< shader::snippet_id > snippets );

protected:
	std::string fragmentShader;
	std::vector< Param::Ptr > params;
	ffglex::FFGLShader shader;
	ffglex::FFGLScreenQuad quad;

	float relativeTime = 0;
	float lastUpdate   = 0;
	int frame          = 0;
	std::chrono::time_point< std::chrono::high_resolution_clock > t_start = std::chrono::high_resolution_clock::now();
	Audio audio;
	utils::Random random;
	std::set< shader::snippet_id > includedSnippets;

	std::string fragmentShaderCodeStart = R"(
		#version 410 core
		out vec4 fragColor;
		uniform vec2 resolution;
		uniform float time;
		uniform float deltaTime;
		uniform float relativeTime;
		uniform int frame;
		uniform float audioVolume;
		uniform float audioBass;
		uniform float audioMed;
		uniform float audioHigh;
		uniform float bpm;
		uniform float phase;
	)";
	std::string vertexShaderCode;
};

template< typename PluginType >
inline PluginInstance Plugin::createPlugin( PluginInfo infos, FFUInt32 type )
{
	return PluginInstance(
		PluginFactory< PluginType >,// Create method
		infos.id.c_str(),           // Plugin unique ID
		infos.name.c_str(),         // Plugin name
		2,                          // API major version number
		1,                          // API minor version number
		infos.majorVersion,         // Plugin major version number
		infos.minorVersion,         // Plugin minor version number
		type,                       // Plugin type
		infos.description.c_str(),  // Plugin description
		infos.about.c_str()         // About
	);
}
