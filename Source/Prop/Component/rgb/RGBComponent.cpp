/*
  ==============================================================================

	RGBComponent.cpp
	Created: 8 May 2020 3:09:02pm
	Author:  bkupe

  ==============================================================================
*/

RGBPropComponent::RGBPropComponent(Prop* prop, var params) :
	PropComponent(prop, "RGB"),
	updateRate(params.getProperty("updateRate", 50)),
	resolution(params.getProperty("resolution", 32)),
	useLayout(params.hasProperty("layout"))
{

	brightness = addFloatParameter("Brightness", "Brightness", params.getProperty("defaultBrightness", .5f), 0, 1);
	streamEnable = addBoolParameter("Stream Colors", "If checked, this will be streaming colors", true);
	invertDirection = addBoolParameter("Invert Direction", "If checked, inverse the order of  the leds", params.getProperty("invertDirection", false));

	if (useLayout)
	{
		var layoutData = params.getProperty("layout", var());
		if (layoutData.size() > 0)
		{
			ledIndexMap.resize(resolution);
			for (int i = 0; i < layoutData.size(); i++) ledIndexMap.set((int)layoutData[i], i);
			for (int i = layoutData.size(); i < resolution; i++) ledIndexMap.set(i, i);
		}
		else
		{
			LOGWARNING("Layout has not the same size as resolution " << layoutData.size() << " < > " << resolution);
		}
	}
}

RGBPropComponent::~RGBPropComponent()
{
}

void RGBPropComponent::handePropConnected()
{
	PropComponent::handePropConnected();
	sendControl("enabled", prop->enabled->boolValue());
}
