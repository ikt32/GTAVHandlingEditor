#pragma once

#include "Memory/HandlingInfo.h"
#include <string>

RTHE::CHandlingData* GetEditHandlingData(bool editStock, bool& hrActive, std::string& vehicleName);
RTHE::CBaseSubHandlingData* GetSubHandlingData(RTHE::CHandlingData* handlingData, RTHE::eHandlingType handlingType);