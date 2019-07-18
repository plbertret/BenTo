/*
  ==============================================================================

	LightBlockLayer.cpp
	Created: 17 Apr 2018 5:10:36pm
	Author:  Ben

  ==============================================================================
*/

#include "LightBlockLayer.h"
#include "ui/LightBlockLayerPanel.h"
#include "ui/LightBlockLayerTimeline.h"
#include "Prop/PropManager.h"
#include "Prop/Cluster/PropClusterGroupManager.h"
#include "Timeline/TimelineBlockSequence.h"

LightBlockLayer::LightBlockLayer(Sequence * s, var params) :
	SequenceLayer(s, "Block Layer"),
	blockClipManager(this)
{
	defaultLayer = addBoolParameter("Default", "If checked, this layer will be the default layer when no layer has the requested prop id", false);
	previewID = addIntParameter("Preview ID", "ID to preview the content", 0, 0, 500);
	previewID->hideInEditor = true;

	blendMode = addEnumParameter("Blend Mode", "The Blend mode of this layer");
	blendMode->addOption("Add", ADD)->addOption("Alpha", ALPHA)->addOption("Mask", MASK);

	filterManager.reset(new PropTargetFilterManager(&((TimelineBlockSequence*)sequence)->clusterGroupManager));
	addChildControllableContainer(filterManager.get());
	filterManager->addFilterManagerListener(this);

	updateLinkedProps();

	Engine::mainEngine->addEngineListener(this);
}

LightBlockLayer::~LightBlockLayer()
{

}

Array<Colour> LightBlockLayer::getColors(Prop * p, double time, var params)
{
	Array<LayerBlock *> blocks = blockClipManager.getBlocksAtTime(time, false);

	int resolution = p->resolution->intValue();

	Array<Colour> result;
	result.resize(resolution);

	if (blocks.size() == 0)
	{
		result.fill(Colours::transparentBlack);
		return result;
	}

	Array<Array<Colour>> clipColors;

	for (auto &b : blocks)
	{
		LightBlockClip * clip = (LightBlockClip *)b;
		clipColors.add(clip->getColors(p, time, params));
	}

	for (int i = 0; i < resolution; i++)
	{
		float r = 0, g = 0, b = 0, a = 0;

		for (int j = 0; j < clipColors.size(); j++)
		{
			if (i >= clipColors[j].size()) continue;

			float ca = clipColors[j][i].getFloatAlpha();
			r += clipColors[j][i].getFloatRed() * ca;
			g += clipColors[j][i].getFloatGreen() * ca;
			b += clipColors[j][i].getFloatBlue() * ca;
			a += ca;
		}

		result.set(i, (Colour::fromFloatRGBA(jmin(r, 1.f), jmin(g, 1.f), jmin(b, 1.f), jmin(a, 1.f))));
	}

	return result;
}

void LightBlockLayer::updateLinkedProps()
{
	if (PropManager::getInstanceWithoutCreating() != nullptr)
	{
		for (auto &p : PropManager::getInstance()->items)
		{
			if (filterManager->getTargetIDForProp(p) >= 0)
			{
				registerLinkedInspectable(p);
			}
			else
			{
				unregisterLinkedInspectable(p);
			}
		}
	}
}

void LightBlockLayer::filtersChanged()
{
	updateLinkedProps();
}

bool LightBlockLayer::paste()
{
	Array<LayerBlock *> p = blockClipManager.addItemsFromClipboard(false);
	if (p.isEmpty()) return SequenceLayer::paste();
	return true;
}

void LightBlockLayer::endLoadFile()
{
	updateLinkedProps();
	Engine::mainEngine->removeEngineListener(this);
}

SequenceLayerPanel * LightBlockLayer::getPanel()
{
	return new LightBlockLayerPanel(this);
}

SequenceLayerTimeline * LightBlockLayer::getTimelineUI()
{
	return new LightBlockLayerTimeline(this);
}

var LightBlockLayer::getJSONData()
{
	var data = SequenceLayer::getJSONData();
	data.getDynamicObject()->setProperty("blocks", blockClipManager.getJSONData());
	data.getDynamicObject()->setProperty("filters", filterManager->getJSONData());
	return data;
}

void LightBlockLayer::loadJSONDataInternal(var data)
{
	SequenceLayer::loadJSONDataInternal(data);
	blockClipManager.loadJSONData(data.getProperty("blocks", var()));
	filterManager->loadJSONData(data.getProperty("filters", var()));
}
