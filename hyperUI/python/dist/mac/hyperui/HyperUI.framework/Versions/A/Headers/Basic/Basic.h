#pragma once

// FPS - one for everything, one for drawing.
// Note that GAME_FRAMERATE/GAME_DRAWING_FRAMERATE will be cast
// to an integer.
// 60
#define GAME_FRAMERATE				60.0
//#define GAME_DRAWING_FRAMERATE		60.0

#define GLOBAL_ANIM_START_TIME	2

#include "Types.h"
#include "UITypes.h"
#include "UIProperties.h"
#include "UICollections.h"
#include "SColorPoint.h"
#include "SBrushPoint.h"
#include "SVertexInfo.h"
#include "SVertex3D.h"
#include "CombinedTransform.h"
#include "UniqueIdGenerator.h"
#include "GlobalIdGenerator.h"
#include "GlobalIdGeneratorInternal.h"
#include "IBaseImage.h"
#include "STouchInfo.h"
#include "Material.h"
#include "AnimSequenceAddon.h"
#include "SColorList.h"
#include "ICustomDragRenderer.h"
#include "ICursorManager.h"
#include "BaseUtils.h"
#include "FormatsManager.h"