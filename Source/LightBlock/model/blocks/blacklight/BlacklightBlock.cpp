#include "BentoEngine.h"

BlacklightBlock::BlacklightBlock(var params) :
	LightBlockModel(getTypeString(), params)
{
	brightness = paramsContainer->addFloatParameter("Brightness", "", 1, 0, 1);
}



float BlacklightBlock::getBlackBrightness(Prop* p, double time, var params){

	float fBrightness = getParamValue<float>(brightness, params);
	return fBrightness;

}

void BlacklightBlock::getColorsInternal(Array<Colour>* result, Prop* p, double time, int id, int resolution, var params)
{

	float fBrightness = getParamValue<float>(brightness, params);
	Colour bBGColor = Colour::fromFloatRGBA(0.5, 0, 1, 1);

	for (int i = 0; i < resolution; i++){
		//result->set(i, bBGColor);
	}

}