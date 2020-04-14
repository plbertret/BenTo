/*
  ==============================================================================

    BentoProp.h
    Created: 5 Mar 2019 11:01:56pm
    Author:  bkupe

  ==============================================================================
*/

#pragma once

#include "../../Prop.h"
#include "Common/Serial/SerialDeviceParameter.h"


class BentoProp :
	public Prop,
	public SerialDevice::SerialDeviceListener
{
public:
	BentoProp(var params);
	virtual ~BentoProp();
	StringParameter * remoteHost;

	const int remotePort = 8888;
	DatagramSocket sender;
	OSCSender oscSender;

	SerialDeviceParameter* serialParam;
	SerialDevice* serialDevice;


	virtual void clearItem() override;

	virtual void setSerialDevice(SerialDevice* d);

	void onControllableFeedbackUpdateInternal(ControllableContainer * cc, Controllable *c) override;

	virtual void serialDataReceived(SerialDevice * d, const var &) override;
	virtual void portRemoved(SerialDevice* d) override;

	virtual void sendColorsToPropInternal() override;

	virtual void uploadBakedData(BakeData data) override;
	virtual void exportBakedData(BakeData data) override;

	virtual void loadBake(StringRef fileName, bool autoPlay = false) override;
	virtual void playBake(float time = 0, bool loop = false) override;
	virtual void pauseBakePlaying() override;
	virtual void resumeBakePlaying() override {}
	virtual void seekBakePlaying(float time) override;
	virtual void stopBakePlaying() override;
	virtual void sendShowPropID(bool value) override;

	static bool uploadProgressCallback(void* prop, int bytesSent, int totalBytes);
	static bool uploadMetaDataProgressCallback(void * prop, int bytesSent, int totalBytes);

	virtual void sendPing() override;
	virtual void powerOffProp() override;
	virtual void restartProp() override;
	virtual void sendWiFiCredentials(String ssid, String pass);

	virtual void sendControllableFeedbackToProp(Controllable* c) override;
	virtual void sendMessageToProp(const OSCMessage& m);

	void  createControllablesForContainer(var data, ControllableContainer* cc);
	Controllable* getControllableForJSONDefinition(const String& name, var def);

	static BentoProp* create(var params) { return new BentoProp(params); }
};