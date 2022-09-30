#pragma once

class BlacklightBlock :
	public LightBlockModel
{
public:
	BlacklightBlock(var params = var());
	~BlacklightBlock() {}

	FloatParameter* brightness;

	float getBlackBrightness(Prop* p, double time, var params);
	void getColorsInternal(Array<Colour>* result, Prop* p, double time, int id, int resolution, var params) override;

	String getTypeString() const override { return "Blacklight"; }
};