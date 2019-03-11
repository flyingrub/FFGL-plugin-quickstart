#pragma once
#include "Plugin.h"
class Effect : public Plugin
{
public:
	Effect();
	~Effect();

	template<typename PluginType>
	static PluginInstance createPlugin(PluginInfo infos);

	FFResult InitGL(const FFGLViewportStruct * vp) override;
	FFResult ProcessOpenGL(ProcessOpenGLStruct * pGL) override;
};

template<typename PluginType>
inline PluginInstance Effect::createPlugin(PluginInfo infos)
{
	return Plugin::createPlugin<PluginType>(infos, FF_EFFECT);
}
