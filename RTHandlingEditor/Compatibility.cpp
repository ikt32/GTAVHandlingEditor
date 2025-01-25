#include "Compatibility.h"

#include "Util/Logger.hpp"

#include <HandlingReplacement.h>
#include <Windows.h>

HMODULE g_HandlingReplacementModule = nullptr;
bool(*g_HR_Enable)(int handle, void** pHandlingData) = nullptr;
bool(*g_HR_Disable)(int handle, void** pHandlingData) = nullptr;
bool(*g_HR_GetHandlingData)(int handle, void** pHandlingDataOriginal, void** pHandlingDataReplaced) = nullptr;

template <typename T>
T CheckAddr(HMODULE lib, const std::string& funcName) {
    FARPROC func = GetProcAddress(lib, funcName.c_str());
    if (!func) {
        LOG(Error, "[Compat] Couldn't get function [{}]", funcName.c_str());
        return nullptr;
    }
    LOG(Debug, "[Compat] Found function [{}]", funcName.c_str());
    return reinterpret_cast<T>(func);
}

void setupHandlingReplacement() {
    LOG(Info, "[Compat] Setting up HandlingReplacement");
    if (g_HandlingReplacementModule &&
        g_HR_Enable && g_HR_Disable && g_HR_GetHandlingData) {
        LOG(Info, "[Compat] HandlingReplacement.asi already loaded");
        return;
    }

    g_HandlingReplacementModule = GetModuleHandle("HandlingReplacement.asi");
    if (!g_HandlingReplacementModule) {
        LOG(Info, "[Compat] HandlingReplacement.asi not found");
        return;
    }

    g_HR_Enable = CheckAddr<bool(*)(int, void**)>(g_HandlingReplacementModule, "HR_Enable");
    g_HR_Disable = CheckAddr<bool(*)(int, void**)>(g_HandlingReplacementModule, "HR_Disable");
    g_HR_GetHandlingData = CheckAddr<bool(*)(int, void**, void**)>(g_HandlingReplacementModule, "HR_GetHandlingData");
}

void setupCompatibility() {
    setupHandlingReplacement();
}

void releaseCompatibility() {
}

bool HR_Enable(int vehicle, void** pHandlingData) {
    if (g_HR_Enable) {
        return g_HR_Enable(vehicle, pHandlingData);
    }
    return false;
}

bool HR_Disable(int vehicle, void** pHandlingData) {
    if (g_HR_Disable) {
        return g_HR_Disable(vehicle, pHandlingData);
    }
    return false;
}

bool HR_GetHandlingData(int vehicle, void** pHandlingDataOriginal, void** pHandlingDataReplaced) {
    if (g_HR_GetHandlingData) {
        return g_HR_GetHandlingData(vehicle, pHandlingDataOriginal, pHandlingDataReplaced);
    }
    return false;
}
