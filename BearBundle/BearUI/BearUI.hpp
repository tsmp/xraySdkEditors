#pragma once

#include "../BearCore/BearCore.hpp"
#include "../BearGraphics/BearGraphics.hpp"

#ifdef BEARUI_EXPORTS
#define BEARUI_API  BEARDLL_EXPORT
#else
#define BEARUI_API  BEARDLL_IMPORT
#define IMGUI_USER_CONFIG "..\imconfig.h"
#endif

#include "imgui/imgui.h"
#undef IMGUI_API

#include "BearUIObject.h"
#include "BearUIViewportBase.h"
#include "BearUIViewport.h"