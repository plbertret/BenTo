/*
  ==============================================================================

	PropUI.cpp
	Created: 10 Apr 2018 7:00:05pm
	Author:  Ben

  ==============================================================================
*/

PropUI::PropUI(Prop* p) :
	BaseItemUI(p),
	viz(p),
	imuRef(nullptr)
{
	itemLabel.setVisible(false);

	acceptedDropTypes.add("LightBlockModel");
	acceptedDropTypes.add("Timeline");
	acceptedDropTypes.add("Script");
	acceptedDropTypes.add("Picture");
	acceptedDropTypes.add("Node");

	idUI.reset(p->globalID->createLabelUI());
	idUI->showLabel = false;

	if (BatteryPropComponent* bat = dynamic_cast<BatteryPropComponent*>(p->getComponent("battery")))
	{
		batteryUI.reset(bat->level->createSlider());
		addAndMakeVisible(batteryUI.get());
	}

	addAndMakeVisible(idUI.get());
	addAndMakeVisible(&viz);

	if (IMUPropComponent* imu = dynamic_cast<IMUPropComponent*>(p->getComponent("imu"))) imuRef = imu->enabled;

	viz.setInterceptsMouseClicks(false, false);

	Prop::Shape shape = p->type->getValueDataAsEnum<Prop::Shape>();

	setSize(shape == Prop::HOOP ? 100 : 50, 100);
}

PropUI::~PropUI()
{
}

void PropUI::paintOverChildren(Graphics& g)
{
	BaseItemUI::paintOverChildren(g);
	if (item->isBaking->boolValue())
	{
		g.fillAll(Colours::black.withAlpha(.3f));

		g.setColour(Colours::orange.darker().withAlpha(.2f));
		g.fillRoundedRectangle(viz.getBounds().removeFromBottom(item->bakingProgress->floatValue() * viz.getHeight()).toFloat(), 2);

		g.setColour(Colours::limegreen.darker().withAlpha(.2f));
		g.fillRoundedRectangle(viz.getBounds().removeFromBottom(item->uploadProgress->floatValue() * viz.getHeight()).toFloat(), 2);

		g.setColour(item->isUploading->boolValue() ? Colours::limegreen : Colours::orange);
		g.drawFittedText(item->isUploading->boolValue() ? "Uploading ..." : "Baking...", getLocalBounds(), Justification::centred, 1);
	}

	g.setColour(item->isConnected->boolValue() ? GREEN_COLOR : BG_COLOR);
	Rectangle<int> r = getMainBounds().translated(0, headerHeight + headerGap).removeFromRight(20).removeFromTop(20).reduced(1);
	g.fillEllipse(r.toFloat().reduced(4));

	if (imuRef != nullptr && imuRef->boolValue())
	{
		g.setColour(YELLOW_COLOR);
		g.drawEllipse(r.toFloat().reduced(2), 1);
	}


	if (isDraggingOver)
	{
		g.fillAll(BLUE_COLOR.withAlpha(.3f));
	}
}

void PropUI::mouseDown(const MouseEvent& e)
{
	BaseItemUI::mouseDown(e);

	if (e.mods.isRightButtonDown())
	{
		LightBlockModelLibrary::showSourcesAndGet(nullptr,[this](ControllableContainer* cc)
			{
				LightBlockColorProvider* p = dynamic_cast<LightBlockColorProvider*>(cc);
				if (p != nullptr) item->activeProvider->setValueFromTarget(p);
			}
		);
	}
	else if (e.mods.isLeftButtonDown())
	{
		if (e.mods.isAltDown())
		{
			item->findPropMode->setValue(true);
		}
	}
}

void PropUI::mouseUp(const MouseEvent& e)
{
	BaseItemUI::mouseUp(e);
	item->findPropMode->setValue(false);
}

void PropUI::resizedInternalHeader(Rectangle<int>& r)
{

}

void PropUI::resizedInternalContent(Rectangle<int>& r)
{

	idUI->setBounds(r.removeFromTop(16).removeFromLeft(30));
	r.removeFromTop(2);

	if (batteryUI != nullptr)
	{
		batteryUI->setBounds(r.removeFromTop(14).reduced(2));
	}

	viz.setBounds(r.reduced(2));
}

void PropUI::controllableFeedbackUpdateInternal(Controllable* c)
{
	if (c == item->isBaking || c == item->bakingProgress || c == item->isUploading || c == item->uploadProgress || c == item->isConnected || c == imuRef) repaint();
	else if (c == item->type)
	{
		Prop::Shape shape = item->type->getValueDataAsEnum<Prop::Shape>();
		setSize(shape == Prop::HOOP ? 100 : 50, 100);
	}
}

void PropUI::itemDropped(const SourceDetails& source)
{
	LightBlockModelUI* modelUI = dynamic_cast<LightBlockModelUI*>(source.sourceComponent.get());

	if (modelUI != nullptr)
	{

		bool shift = KeyPress::isKeyCurrentlyDown(16);
		if (shift)
		{
			PopupMenu m;
			m.addItem(-1, "Default");
			m.addSeparator();
			int index = 1;
			for (auto& p : modelUI->item->presetManager.items) m.addItem(index++, p->niceName);
			m.showMenuAsync(PopupMenu::Options(), [this, modelUI](int result)
				{
					if (result >= 1)
					{
						LightBlockColorProvider* provider = modelUI->item->presetManager.items[result - 1];
						item->activeProvider->setValueFromTarget(provider);
					}
				}
			);
		}
		else
		{
			item->activeProvider->setValueFromTarget(modelUI->item);
		}

	}

	BaseItemUI::itemDropped(source);

}
