#pragma once

#include "msgpack.h"
#include "wintab.h"
#define PACKETDATA	(PK_X | PK_Y | PK_BUTTONS | PK_NORMAL_PRESSURE | PK_CURSOR | PK_TANGENT_PRESSURE | PK_ORIENTATION )
#define PACKETMODE	PK_BUTTONS
#include "pktdef.h"
#include "Utils.h"
