/*
  ==============================================================================

    WasmBlock.cpp
    Created: 9 Nov 2021 6:06:13pm
    Author:  bkupe

  ==============================================================================
*/

#include "Prop/PropIncludes.h"
#include "WasmBlock.h"

WasmBlock::WasmBlock(var params) :
	LightBlockModel(getTypeString(), params),
	Thread("WasmCompile")
{
	scriptFile = addFileParameter("Script", "Path to the script");
	scriptFile->fileTypeFilter = "*.ts";

	compileType = addEnumParameter("Compile Type", "");
	compileType->addOption("Optimized", COMPILE_OPTIMIZED)->addOption("Debug", COMPILE_DEBUG)->addOption("Tiny", COMPILE_TINY);

	lowMemory = addBoolParameter("Low Memory", "", false);
	autoCompile = addBoolParameter("Auto Compile", "", true);
	compileTrigger = addTrigger("Compile", "Compiles the script");
	uploadToPropsTrigger = addTrigger("Upload to Props", "");
	autoUpload = addBoolParameter("Auto Upload", "", true);
	loadOnPropsTrigger = addTrigger("Load on Props", "");
	autoLaunch = addBoolParameter("Auto Launch", "", true);
	stopOnPropsTrigger = addTrigger("Stop on Props", "");
}

WasmBlock::~WasmBlock()
{
}

void WasmBlock::checkAutoCompile()
{
	if (!autoCompile->boolValue()) return;
	File f = scriptFile->getFile();
	if (!f.exists()) return;
	Time t = f.getLastModificationTime();
	if (t > lastModTime) startThread();
	lastModTime = t;

}

void WasmBlock::compile()
{
	File f = scriptFile->getFile();
	if (!f.exists()) return;
	File folder = f.getParentDirectory();

	File wf = getWasmFile();
	if (wf.exists()) wf.deleteFile();

	//bool silentMode = false;
	//bool result = true;

	CompileType t = compileType->getValueDataAsEnum<CompileType>();
	String options = "";
	switch (t)
	{
	case COMPILE_DEBUG:
		options = "--debug";
		break;

	case COMPILE_OPTIMIZED:
		options = "-O3s --noAssert";
		break;

	case COMPILE_TINY:
		options = "-O3z --noAssert --runtime stub --use abort =";
		break;
	}

	if (lowMemory->boolValue())
	{
		options += " --lowMemoryLimit";
	}

	//String buildCommand = "npm run build";
	String arguments = f.getFullPathName() + " -b " + folder.getChildFile(shortName + ".wasm").getFullPathName() + " " + /*" -t app.wat " +*/ options;

	String command = File::getSpecialLocation(File::userDocumentsDirectory).getChildFile("Bento/wasm/wasmcompiler.exe").getFullPathName() + " " + arguments;

	NLOG(niceName, command);
	ChildProcess p;
	p.start(command);
	String pResult = p.readAllProcessOutput().replace("\r", "");

	//if (silentMode) WinExec(command.getCharPointer(), SW_HIDE);
	//else result = system(command.getCharPointer());

	if (pResult.isEmpty())
	{
		File nwf = getWasmFile();
		NLOG(niceName, "Script has been compiled successfully to " + nwf.getFileName());
		if (autoUpload->boolValue())
		{
			uploadToPropsTrigger->trigger();
			if (autoLaunch->boolValue())
			{
				std::function<void()> func = std::bind(&Trigger::trigger, loadOnPropsTrigger);
				Timer::callAfterDelay(200, func);
			}
		}
	}
	else
	{
		NLOGERROR(niceName, "Error compiling script : \n" << pResult);
	}
}

File WasmBlock::getWasmFile()
{
	File f = scriptFile->getFile();
	if (!f.exists()) return File();
	return f.getParentDirectory().getChildFile(shortName + ".wasm");
}

void WasmBlock::loadScriptOnProp(Prop* p)
{
	p->sendControlToProp("scripts.load", shortName);
}

void WasmBlock::stopScriptOnProp(Prop* p)
{
	p->sendControlToProp("scripts.stop", shortName);
}

void WasmBlock::run()
{
	compile();
}

void WasmBlock::onContainerParameterChangedInternal(Parameter* p)
{
	LightBlockModel::onContainerParameterChangedInternal(p);
	if (p == scriptFile)
	{
		File f = scriptFile->getFile();
		if (f.existsAsFile()) lastModTime = f.getLastModificationTime();
	}
}

void WasmBlock::onContainerTriggerTriggered(Trigger* t)
{
	if (t == compileTrigger) startThread();
	else if (t == uploadToPropsTrigger)
	{
		File f = getWasmFile();
		if (!f.existsAsFile())
		{
			NLOGWARNING(niceName, "Script " << f.getFileName() << "doesn't exists");
			return;
		}

		for (auto& p : PropManager::getInstance()->items)
		{
			p->addFileToUpload(f);
		}
	}
	else if (t == loadOnPropsTrigger)
	{
		for (auto& p : PropManager::getInstance()->items)
		{
			p->sendControlToProp("scripts.load", shortName);
		}
	}
	else if (t == stopOnPropsTrigger)
	{
		for (auto& p : PropManager::getInstance()->items)
		{
			p->sendControlToProp("scripts.stop", shortName);
		}
	}
}

void WasmBlock::handleEnterExit(bool enter, Array<Prop*> props)
{
	for (auto& p : props)
	{
		if (enter) loadScriptOnProp(p);
		else stopScriptOnProp(p);
	}
}


void WasmBlock::getColorsInternal(Array<Colour>* result, Prop* p, double time, int id, int resolution, var params)
{
	result->fill(Colours::black.withAlpha(.2f));
}