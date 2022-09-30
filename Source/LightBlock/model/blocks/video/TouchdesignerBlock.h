#pragma once

class TouchdesignerBlock :
	public TextureBlock,
	public SharedTextureReceiver::Listener,
	public SharedTextureManager::Listener,
	public ParameterListener

{
public:
	TouchdesignerBlock(var params = var());
	~TouchdesignerBlock();


	StringParameter* textureName;
	SharedTextureReceiver* receiver;
	Array<Colour> colors;
	int lastResolution;

	Image tmpImage;


	virtual Array<WeakReference<Controllable>> getModelParameters() override;

	void setupReceiver();
	Image getImage() override;

	//void onContainerParameterChangedInternal(Parameter* p) override;

	Array<Colour> getColors(Prop* p, double time, var params) override;

	// Inherited via Listener
	virtual void textureUpdated(SharedTextureReceiver*) override;
	virtual void connectionChanged(SharedTextureReceiver*) override;
	virtual void receiverRemoved(SharedTextureReceiver* r) override;

	String getTypeString() const override { return "Touchdesigner"; }

	static LiveFeedBlock* create(var params) { return new LiveFeedBlock(params); }

	void parameterValueChanged(Parameter* p);

	var getJSONData() override;
	void loadJSONDataInternal(var data) override;
	void onControllableFeedbackUpdateInternal(ControllableContainer* cc, Controllable* c) override;
	void childStructureChanged(ControllableContainer* cc) override;
};