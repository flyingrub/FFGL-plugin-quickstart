#include "Add.h"

std::string mfFragShader = R"(
void main()
{
	//get the two fragments to mix
	vec4 colorDest = texture( textureDest, uvDest );
	vec4 colorSrc  = texture( textureSrc, uvSrc );

	//here we add the colorSrc r,g,b,a pixel value to the colorDest pixel value according to the mixVal value
	vec4 mix = colorDest + colorSrc * mixVal;

	//Here we use the built-in function min(val1,val2) to get the minimum between val1 and val2 and always keep output pixel value between 0.0 and 1.0
	fragColor = min( mix, 1.0 );
}
)";

Add::Add()
{
	setFragmentShader(mfFragShader);
}

Add::~Add()
{
}
