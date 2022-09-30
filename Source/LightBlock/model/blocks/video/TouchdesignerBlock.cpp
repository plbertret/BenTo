#include "TouchdesignerBlock.h"


TouchdesignerBlock::TouchdesignerBlock(var params) :
	TextureBlock(getTypeString(), params),
	lastResolution(25),
	receiver(nullptr)
{


	textureName = paramsContainer->addStringParameter("Texture Name", "The Spout / Syphon name of the texture", "");

	textureName->addParameterListener(this);
	setupReceiver();

	SharedTextureManager::getInstance()->addListener(this);
}




Array<WeakReference<Controllable>> TouchdesignerBlock::getModelParameters()
{
	if (paramsContainer == nullptr) return Array<WeakReference<Controllable>>();
	return paramsContainer->getAllControllables();
}


TouchdesignerBlock::~TouchdesignerBlock()
{

	if (SharedTextureManager::getInstanceWithoutCreating() != nullptr)
	{
		if (receiver != nullptr)
		{
			receiver->removeListener(this);
			SharedTextureManager::getInstance()->removeReceiver(receiver);
			receiver = nullptr;
		}

		SharedTextureManager::getInstance()->removeListener(this);
	}

}

void TouchdesignerBlock::setupReceiver()
{
	if (receiver != nullptr)
	{
		receiver->removeListener(this);
		SharedTextureManager::getInstance()->removeReceiver(receiver);
	}

	String textName = textureName->stringValue();

	receiver = nullptr;
	receiver = SharedTextureManager::getInstance()->addReceiver(textName);

	if (receiver != nullptr)
	{
		receiver->addListener(this);
		//receiver->setUseCPUImage(true);
		//receiver->createReceiver();
	}

}

Image TouchdesignerBlock::getImage()
{
	if (receiver == nullptr || isClearing) return Image();
	else  return receiver->getImage();
}



Array<Colour> TouchdesignerBlock::getColors(Prop* p, double time, var params)
{
	if (Engine::mainEngine->isClearing) return Array<Colour>();

	if (!inputIsLive->boolValue()) return LightBlockModel::getColors(p, time, params);

	if (colors.size() == 0) {
		return Array<Colour>();
	}


	int resolution = p->resolution->intValue();
	if (resolution > 0) {
		lastResolution = resolution;
	}

	return colors;
}


void TouchdesignerBlock::textureUpdated(SharedTextureReceiver*)
{
	Image tex = getImage();

	colors.resize(lastResolution);
	colors.fill(Colours::black);

	if (tex.isNull()) return;

	int width = tex.getWidth();
	int height = tex.getHeight();
	const Image::BitmapData data(tex, 0, 0, tex.getWidth(), tex.getHeight());
	for (int i = lastResolution; i > 0; i--)
	{
		int x = jlimit<int>(0, tex.getWidth() - 1, (lastResolution - i) * width / lastResolution);
		Colour color = data.getPixelColour(x, tex.getHeight() / 2);
		colors.setUnchecked(i - 1, color);

	}
}

void TouchdesignerBlock::connectionChanged(SharedTextureReceiver*)
{
	inputIsLive->setValue(receiver->isConnected);
}

void TouchdesignerBlock::receiverRemoved(SharedTextureReceiver* r)
{
	if (r == receiver) receiver = nullptr;
}


void TouchdesignerBlock::parameterValueChanged(Parameter* p){
	if (p == textureName) {
		setupReceiver();
	}
}




var TouchdesignerBlock::getJSONData()
{
	var data = LightBlockModel::getJSONData();
	if (paramsContainer != nullptr)
	{
		var cData = paramsContainer->getJSONData();
		if (cData.getDynamicObject()->getProperties().size() > 0) data.getDynamicObject()->setProperty("modelParams", cData);
	}
	if (presetManager.items.size() > 0) data.getDynamicObject()->setProperty("presets", presetManager.getJSONData());
	return data;
}

void TouchdesignerBlock::loadJSONDataInternal(var data)
{
	LightBlockModel::loadJSONDataInternal(data);
	if (paramsContainer != nullptr) paramsContainer->loadJSONData(data.getProperty("modelParams", var()));
	presetManager.loadJSONData(data.getProperty("presets", var()));
}

void TouchdesignerBlock::onControllableFeedbackUpdateInternal(ControllableContainer* cc, Controllable* c)
{
	if (Engine::mainEngine->isClearing) return;
	if (cc == paramsContainer.get()) providerListeners.call(&ProviderListener::providerParameterValueUpdated, this, dynamic_cast<Parameter*>(c));
}

void TouchdesignerBlock::childStructureChanged(ControllableContainer* cc)
{
	if (cc == paramsContainer.get()) providerListeners.call(&ProviderListener::providerParametersChanged, this);
}

