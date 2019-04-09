#pragma once
#include <vector>
#include <string>
#include <FFGLSDK.h>
#include "../helpers/Utils.h"

class Param {
protected:
	float value;
	std::string name;
	FFUInt32 type;
public:
	Param() : Param("") {}
	Param(std::string name) : Param("", 0) {}
	Param(std::string name, float value) : Param(name, FF_TYPE_STANDARD, value) {}
	Param(std::string name, FFUInt32 type, float value) : name(name), type(type), value(value) {}

	virtual void setValue(float v) { value = v; }

	virtual float getValue() { return value; }

	std::string getName() { return name; }

	FFUInt32 getType() { return type; }
};

class ParamRange : public Param {
public:
	struct Range {
		float min = 0.0f, max = 1.0f;
	};
	ParamRange(std::string name, float value, Range range) :
		Param(name, FF_TYPE_STANDARD, value), range(range) {}
	float getValue() override
	{
		return utils::map(value, 0.0, 1.0, range.min, range.max);
	}
private:
	Range range;
};

class ParamOption : public Param {

public:
	typedef std::vector<std::string> Options;

	Options options;

	ParamOption(std::string name, std::vector<std::string> options) :
		ParamOption(name, options, 0) {}
	ParamOption(std::string name, std::vector<std::string> options, int defaultOption) :
		Param(name, FF_TYPE_OPTION, defaultOption), options(options)
	{
		setValue(defaultOption);
	}
	void setValue(float _value) override
	{
		options.size() <= _value ? value = 0 : value = _value;
		currentOption = options[value];
	}
	bool isCurrentOption(std::string option)
	{
		return option.compare(currentOption) == 0;
	}
private:
	std::string currentOption;
};

class ParamEvent : public Param {
public:
	ParamEvent(std::string name) : Param(name, FF_TYPE_EVENT, 0) {}
};

class ParamTrigger : public ParamEvent {
public:
	ParamTrigger(std::string name) : ParamEvent(name) {}
	
	virtual void setValue(float _value) override
	{
		bool current = _value == 1;
		bool previous = value == 1;
		tiggerValue = current && !previous;
	}

	float getValue() override { return tiggerValue; }
private:
	bool tiggerValue;
};

class ParamBool : public Param {
public:
	ParamBool(std::string name) : ParamBool(name, false) {}
	ParamBool(std::string name, bool defaultValue) : Param(name, FF_TYPE_BOOLEAN, defaultValue) {}
};

class ParamText : public Param {
public:
	std::string text;
	ParamText(std::string name) : ParamText(name, "") {}
	ParamText(std::string name, std::string text) : Param(name, FF_TYPE_TEXT, 0), text(text) {}
};
