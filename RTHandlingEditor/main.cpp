/*
THIS FILE IS A PART OF GTA V SCRIPT HOOK SDK
http://dev-c.com
(C) Alexander Blade 2015
*/

#include "inc\main.h"
#include "script.h"
#include "keyboard.h"
#include "Util/Logger.hpp"
#include "Constants.h"
#include "Memory/Versions.h"

BOOL APIENTRY DllMain(HMODULE hInstance, DWORD reason, LPVOID lpReserved)
{
    logger.SetFile(Constants::LogFile);
    logger.SetMinLevel(LogLevel::DEBUG);
    
    switch (reason)
    {
        case DLL_PROCESS_ATTACH: {
            int scriptingVersion = getGameVersion();
            logger.Clear();
            logger.Write(INFO, "Handling Editor %s (built %s %s)", Constants::DisplayVersion, __DATE__, __TIME__);
            logger.Write(INFO, "Game version " + eGameVersionToString(scriptingVersion));
            if (scriptingVersion < G_VER_1_0_1604_0_STEAM) {
                logger.Write(WARN, "Unsupported game version! Update your game.");
            }
            scriptRegister(hInstance, ScriptMain);
            keyboardHandlerRegister(OnKeyboardMessage);
            break;
        }
        case DLL_PROCESS_DETACH: {
            scriptUnregister(hInstance);
            keyboardHandlerUnregister(OnKeyboardMessage);
            break;
        }
    }
    return TRUE;
}
