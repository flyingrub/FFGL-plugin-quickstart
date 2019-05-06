#pragma once
#include "../api/Effect.h"
using namespace ffglex;

class Bloom : public Effect
{
public:
	Bloom();
	FFResult init() override;
	FFResult Bloom::render( ProcessOpenGLStruct* inputTextures ) override;
	void clean() override;
	~Bloom();

private:
	ParamRange::Ptr threshold;
	ParamRange::Ptr radius;
	ParamRange::Ptr intensity;
	ParamBool::Ptr hq;
	ParamBool::Ptr antiFlicker;
	ParamRange::Ptr jitter;
	FFGLShader downSampleFilter;
	FFGLShader upSampleFilter;
	FFGLShader final;
};