/*
  ==============================================================================

    PropManager.h
    Created: 10 Apr 2018 6:59:29pm
    Author:  Ben

  ==============================================================================
*/

#pragma once

#include "Prop.h"
#include "PropFamily.h"

class PropManager :
	public BaseManager<Prop>,
	public OSCReceiver::Listener<OSCReceiver::RealtimeCallback>,
	public Prop::PropListener
{
public:
	juce_DeclareSingleton(PropManager, true)
	
	PropManager();
	~PropManager();

	OSCSender sender;
	OSCReceiver receiver;

	ControllableContainer familiesCC;
	OwnedArray<PropFamily> families;
	Factory<Prop> factory;
	
	const int localPort = 10000;

	BoolParameter * sendFeedback;

	Trigger * detectProps;
	Trigger * autoAssignIdTrigger;
	Trigger * bakeAll;
	Trigger* powerOffAll;
	Trigger* resetAll;
	Trigger* clearAll;

	StringParameter* fileName;
	Trigger* loadAll;
	Trigger* playAll;
	Trigger* stopAll;
	BoolParameter* loop;

	BoolParameter * bakeMode;


	void setupReceiver();

	Prop * getPropWithHardwareId(const String &hardwareId);
	Prop * getPropWithId(int id, Prop * excludeProp = nullptr);

	PropFamily * getFamilyWithName(StringRef familyName);

	void onContainerTriggerTriggered(Trigger * t) override;
	void onContainerParameterChanged(Parameter * p) override;
	void onControllableFeedbackUpdate(ControllableContainer * cc, Controllable *c) override;

	void addItemInternal(Prop * p, var) override;
	void removeItemInternal(Prop * p) override;

	void clear() override;

	int getFirstAvailableID();
	void propIDChanged(Prop * p, int previousID) override;

	// Inherited via Listener
	virtual void oscMessageReceived(const OSCMessage & message) override;

};