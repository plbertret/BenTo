class LedStripComponent;

class LedStripLayer :
    public Component
{
public:
    enum Type
    {
        System,
        Stream,
        Bake,
        ScriptType //need to not be "Script" to avoid Class confusion
    };

    enum BlendMode
    {
        Add,
        Multiply,
        Max,
        Min,
        Alpha
    };

    LedStripLayer(const String &name, Type t, LedStripComponent * strip);
    ~LedStripLayer();

    void initColors();

    LedStripComponent * strip;
    int numLeds;
    Type type;
    Parameter * blendMode;
    
    Color * colors;

    //Helper functions
    void clearColors();
    void fillAll(Color c);
    void fillRange(Color c, float start, float end, bool clear = true);
    void point(Color c, float pos, float radius, bool clear = true);
    
    LinkScriptFunctionsStart
    LinkScriptFunction(LedStripLayer, clear,v,)
    LinkScriptFunction(LedStripLayer, fillAll,v,i)
    LinkScriptFunction(LedStripLayer, fillRange,v,iff)
    LinkScriptFunction(LedStripLayer, point,v,iff)
    LinkScriptFunctionsEnd

    DeclareScriptFunctionVoid0(LedStripLayer, clear) { clearColors(); }
    DeclareScriptFunctionVoid1(LedStripLayer, fillAll, uint32_t) { fillAll(arg1); }
    DeclareScriptFunctionVoid3(LedStripLayer, fillRange, uint32_t, float, float) { fillRange(arg1, arg2, arg3); }
    DeclareScriptFunctionVoid3(LedStripLayer, point, uint32_t, float, float) { point(arg1, arg2, arg3); }
};