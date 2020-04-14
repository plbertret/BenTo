#include "RGBLedsManager.h"

RGBLedsManager::RGBLedsManager() : Component("rgb"),
                                   globalBrightness(1)
{
}

void RGBLedsManager::init()
{
#ifdef LED_SEPARATE_CHANNELS
  for(int i=0;i < LED_COUNT; i++)
  {
    const RGBLedPins l = rgbLedPins[i];
    pinMode(l.rPin,OUTPUT);
    pinMode(l.gPin,OUTPUT);
    pinMode(l.bPin,OUTPUT);
    
    int startChannel = i*3;
    ledcSetup(startChannel, LED_PWM_FREQUENCY, LED_PWM_RESOLUTION);
    ledcSetup(startChannel+1, LED_PWM_FREQUENCY, LED_PWM_RESOLUTION);
    ledcSetup(startChannel+2, LED_PWM_FREQUENCY, LED_PWM_RESOLUTION);
    
    // attach the channel to the GPIO2 to be controlled
    ledcAttachPin(l.rPin, startChannel);
    ledcAttachPin(l.gPin, startChannel+1);
    ledcAttachPin(l.bPin, startChannel+2);
  }
#else

#ifdef LED_EN_PIN
    pinMode(LED_EN_PIN, OUTPUT); //enable LEDs
    digitalWrite(LED_EN_PIN, HIGH);
#endif
#if defined LED_CLK_PIN
    FastLED.addLeds<LED_TYPE, LED_DATA_PIN, LED_CLK_PIN, LED_COLOR_ORDER>(leds, LED_COUNT).setCorrection(TypicalLEDStrip);
#else
    FastLED.addLeds<LED_TYPE, LED_DATA_PIN, LED_COLOR_ORDER>(leds, LED_COUNT).setCorrection(TypicalLEDStrip);
#endif

#if defined LED2_TYPE
#if defined LED2_CLK_PIN
    FastLED.addLeds<LED2_TYPE, LED2_DATA_PIN, LED2_CLK_PIN, LED2_COLOR_ORDER>(leds, LED2_COUNT).setCorrection(TypicalLEDStrip);
#else
    FastLED.addLeds<LED2_TYPE, LED2_DATA_PIN, LED2_COLOR_ORDER>(leds, LED2_COUNT).setCorrection(TypicalLEDStrip);
#endif //LED2_CLK
#endif //LED2

#endif

#ifdef USE_PREFERENCES
    prefs.begin(name.c_str());
    setBrightness(prefs.getFloat("brightness", globalBrightness), false);
    prefs.end();
#elif defined USE_SETTINGS_MANAGER
//init once with a json if it doesn't exist yet
    prefs.readSettings(String("/" + name + ".json").c_str());
    float brightness = prefs.getFloat("brightness", globalBrightness);
    prefs.loadJson(String("{\"brightness\":\""+String(brightness)+"\"}").c_str());
    prefs.writeSettings(String("/" + name + ".json").c_str());

//actually read the data
    prefs.readSettings(String("/" + name + ".json").c_str());
    setBrightness(prefs.getFloat("brightness", globalBrightness), false);
#endif
}

void RGBLedsManager::update()
{
#ifdef LED_SEPARATE_CHANNELS
    for(int i=0;i<LED_COUNT;i++)
    {
        int startChannel = i*3;
        ledcWrite(startChannel, map(leds[i].r,0,255,1023,0));
        ledcWrite(startChannel+1, map(leds[i].g,0,255,1023,0));
        ledcWrite(startChannel+2, map(leds[i].b,0,255,1023,0));
    }
#else
    FastLED.show();
#endif
}

void RGBLedsManager::setBrightness(float value, bool save)
{
    globalBrightness = min(max(value, 0.f), 1.f);

 #ifndef LED_SEPARATE_CHANNELS
    FastLED.setBrightness((int)(globalBrightness * 60));
    FastLED.show();
#endif
    if (save)
    {
#ifdef USE_PREFERENCES
        prefs.begin(name.c_str());
        prefs.putFloat("brightness", globalBrightness);
        prefs.end();
#elif defined USE_SETTINGS_MANAGER
        prefs.readSettings(String("/" + name + ".json").c_str());
        prefs.setFloat("brightness", globalBrightness);
#endif
    }
}

bool RGBLedsManager::handleCommand(String command, var *data, int numData)
{
    NDBG("Handle command : " + command);
    if (checkCommand(command, "brightness", numData, 1))
    {
        setBrightness(data[0].floatValue());
        return true;
    }
    else if (checkCommand(command, "fill", numData, 3))
    {
        CRGB c((int)(data[0].floatValue() * 255), (int)(data[1].floatValue() * 255), (int)(data[2].floatValue() * 255));
        if (numData >= 4)
            c.nscale8((int)(data[3].floatValue() * 255));
        sendEvent(RGBLedsEvent(RGBLedsEvent::ASK_FOCUS));
        fillAll(c);
        return true;
    }
    else if (checkCommand(command, "range", numData, 5))
    {
        bool hasAlpha = numData >= 6;
        CRGB c((int)(data[0].floatValue() * 255), (int)(data[1].floatValue() * 255), (int)(data[2].floatValue() * 255));
        if (hasAlpha)
            c.nscale8((int)(data[3].floatValue() * 255));
        fillRange(c, data[hasAlpha ? 4 : 3].floatValue(), data[hasAlpha ? 5 : 4].floatValue());
        sendEvent(RGBLedsEvent(RGBLedsEvent::ASK_FOCUS));
        return true;
    }
    else if (checkCommand(command, "point", numData, 5))
    {
        bool hasAlpha = numData >= 6;
        CRGB c((int)(data[0].floatValue() * 255), (int)(data[1].floatValue() * 255), (int)(data[2].floatValue() * 255));
        if (hasAlpha)
            c.nscale8((int)(data[3].floatValue() * 255));
        point(c, data[hasAlpha ? 4 : 3].floatValue(), data[hasAlpha ? 5 : 4].floatValue());
        sendEvent(RGBLedsEvent(RGBLedsEvent::ASK_FOCUS));
        return true;
    }

    return false;
}

//Helpers
void RGBLedsManager::clear()
{
 #ifdef LED_SEPARATE_CHANNELS
    for(int i=0;i<LED_COUNT;i++) leds[i] = CRGB::Black;
 #else
   FastLED.clear();
#endif
}

void RGBLedsManager::fillAll(CRGB c)
{
    fillRange(c, 0, 1);
}

void RGBLedsManager::fillRange(CRGB c, float start, float end, bool doClear)
{
    if (doClear)
        clear();
    LedHelpers::fillRange(leds, LED_COUNT, c, start, end);
}

void RGBLedsManager::point(CRGB c, float pos, float fade, bool doClear)
{
    if (doClear)
        clear();
    LedHelpers::point(leds, LED_COUNT, c, pos, fade);
}

void RGBLedsManager::setLed(int index, CRGB c)
{
    if (index < 0 || index >= LED_COUNT)
        return;
    leds[index] = c;
}
