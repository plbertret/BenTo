#pragma once

#include "../../common/Common.h"
#include "LedHelpers.h"

class RGBLedsEvent
{
public:
    enum Type { ASK_FOCUS, BrightnessStatus, TYPES_MAX };
    static const String eventNames[TYPES_MAX];

    RGBLedsEvent(Type t, float * data = nullptr, int numData = 0) : type(t), data(data), numData(numData) {}

    Type type;
    float * data;
    int intData;
    int numData;
};

class RGBLedsManager : public Component, public EventBroadcaster<RGBLedsEvent>
{
public:
    RGBLedsManager();

#ifdef LED_COUNT
    CRGB leds[LED_COUNT];
    #ifdef LED_USE_DMX
        DMXESPSerial dmx;
        const int updateBreakTime = 20; //40ms break time
        long timeSinceLastSend;
    #endif
#endif

    float globalBrightness;
    CRGB temperature;
    bool ledEnabled;

    void init();
    void update();
    void shutdown();

    void setBrightness(float value, bool save = false);
    void setTemperature(uint8_t r, uint8_t g, uint8_t b, bool save = false);
    bool handleCommand(String command, var *data, int numData) override;

    void setLedEnabled(bool val);

    //Helpers
    void clear();
    void fillAll(CRGB c);
    void fillRange(CRGB c, float start, float end, bool clear = true);
    void point(CRGB c, float pos, float radius, bool clear = true);

    void setLed(int index, CRGB c);

private:
#ifdef USE_PREFERENCES
    Preferences prefs;
#elif defined USE_SETTINGS_MANAGER
    SettingsManager prefs;
#endif
};
