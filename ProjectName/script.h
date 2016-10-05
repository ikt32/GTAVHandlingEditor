/*
THIS FILE IS A PART OF GTA V SCRIPT HOOK SDK
http://dev-c.com
(C) Alexander Blade 2015
*/

#pragma once

#include "..\..\ScriptHookV_SDK\inc\natives.h"
#include "..\..\ScriptHookV_SDK\inc\types.h"
#include "..\..\ScriptHookV_SDK\inc\enums.h"

#include "..\..\ScriptHookV_SDK\inc\main.h"

void ScriptMain();

template <typename T>
T getHandlingValue(Vehicle veh, const int valueOffset) {
	const uint64_t address = mem.GetAddressOfEntity(veh);
	uint64_t handlingPtr = *reinterpret_cast<uint64_t *>(address + handlingOffset);
	return *reinterpret_cast<T *>(handlingPtr + valueOffset);
}

template <typename T>
void setHandlingValue(Vehicle veh, int valueOffset, T val) {
	const uint64_t address = mem.GetAddressOfEntity(veh);
	uint64_t handlingPtr = *reinterpret_cast<uint64_t *>(address + handlingOffset);
	*reinterpret_cast<T *>(handlingPtr + valueOffset) = val;
}
