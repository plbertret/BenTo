
#pragma once
#include "../common/Common.h"
#include "wasmFunctions.h"

#define SCRIPT_MAX_SIZE 4096

#define WASM_STACK_SLOTS    2048
#define NATIVE_STACK_SIZE   (32*1024)

#define WASM_MEMORY_LIMIT   4096

class ScriptManager : public Component {
public:
    ScriptManager();
    ~ScriptManager(){}

    static ScriptManager * instance;
    unsigned char scriptData[SCRIPT_MAX_SIZE];
    long scriptSize;

    bool isRunning;
    IM3Runtime runtime;
    IM3Environment env;
    IM3Function updateFunc;
    IM3Function stopFunc;
    TaskHandle_t taskHandle;
    
    void init();
    void update();

    void launchScript(String name);

    void launchWasm();
    void shutdown();

    void stop();

    void launchWasmTask(void*);

    static void launchWasmTaskStatic(void*);
    static M3Result LinkArduino  (IM3Runtime runtime);
    
    bool handleCommand(String command, var *data, int numData) override;
};