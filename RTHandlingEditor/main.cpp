#include "script.h"

#include "Constants.h"
#include "keyboard.h"

#include "Memory/Versions.h"

#include "Util/FileVersion.h"
#include "Util/Logger.hpp"
#include "Util/Paths.h"

#include <inc/main.h>

namespace fs = std::filesystem;

void InitializePaths(HMODULE hInstance) {
    Paths::SetOurModuleHandle(hInstance);

    auto localAppDataPath = Paths::GetLocalAppDataPath();
    auto localAppDataModPath = localAppDataPath / Constants::ModDir;
    std::string originalModPath = Paths::GetModuleFolder(hInstance) + std::string("\\") + Constants::ModDir;
    Paths::SetModPath(originalModPath);

    bool useAltModPath = false;
    if (std::filesystem::exists(localAppDataModPath)) {
        useAltModPath = true;
    }

    std::string modPath;
    std::string logFile;

    // Use LocalAppData if it already exists.
    if (useAltModPath) {
        modPath = localAppDataModPath.string();
        logFile = (localAppDataModPath / (Paths::GetModuleNameWithoutExtension(hInstance) + ".log")).string();
    }
    else {
        modPath = originalModPath;
        logFile = modPath + std::string("\\") + Paths::GetModuleNameWithoutExtension(hInstance) + ".log";
    }

    Paths::SetModPath(modPath);

    if (!fs::is_directory(modPath) || !fs::exists(modPath)) {
        fs::create_directories(modPath);
    }

    logger.SetFile(logFile);
    logger.Clear();

    if (logger.Error()) {
        modPath = localAppDataModPath.string();
        logFile = (localAppDataModPath / (Paths::GetModuleNameWithoutExtension(hInstance) + ".log")).string();
        fs::create_directories(modPath);

        Paths::SetModPath(modPath);
        logger.SetFile(logFile);

        fs::copy(fs::path(originalModPath), localAppDataModPath, fs::copy_options::update_existing | fs::copy_options::recursive);

        // Fix perms
        for (auto& path : fs::recursive_directory_iterator(localAppDataModPath)) {
            try {
                fs::permissions(path, fs::perms::all);
            }
            catch (std::exception& e) {
                logger.Write(ERROR, "Failed to set permissions on [%s]: %s.", path.path().string().c_str(), e.what());
            }
        }

        logger.ClearError();
        logger.Clear();
        logger.Write(WARN, "Copied to [%s] from [%s] due to read/write issues.", modPath.c_str(), originalModPath.c_str());
    }
}

BOOL APIENTRY DllMain(HMODULE hInstance, DWORD reason, LPVOID lpReserved)
{
    logger.SetMinLevel(LogLevel::DEBUG);
    
    switch (reason)
    {
        case DLL_PROCESS_ATTACH: {
            InitializePaths(hInstance);
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
