#pragma once

#include "HandlingItem.h"
#include <inc/types.h>

void setHandling(Vehicle vehicle, const RTHE::CHandlingDataItem& handlingDataItem);

// handlingName is not set!
RTHE::CHandlingDataItem getHandling(Vehicle vehicle);
