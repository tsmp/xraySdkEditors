#include "DX12PCH.h"

#define RENDER_BEGIN_CLASS_REGISTRATION1(Name,...) extern bsize Name ## Counter;
#define RENDER_BEGIN_CLASS_REGISTRATION2(Name,Parent,...) RENDER_BEGIN_CLASS_REGISTRATION1(Name)

#define RENDER_BEGIN_CLASS_REGISTRATION1_WITHOUT_FACTORY(Name,...)
#define RENDER_BEGIN_CLASS_REGISTRATION2_WITHOUT_FACTORY(Name,Parent,...) 
#include "..\BearGraphics\BearTemplate\BearGraphicsObjectsList.h"

#define RENDER_BEGIN_CLASS_REGISTRATION1(Name,...)  bsize DX12Stats::Count ## Name(){return Name ## Counter;}
#define RENDER_BEGIN_CLASS_REGISTRATION2(Name,Parent,...) RENDER_BEGIN_CLASS_REGISTRATION1(Name)

#define RENDER_BEGIN_CLASS_REGISTRATION1_WITHOUT_FACTORY(Name,...)
#define RENDER_BEGIN_CLASS_REGISTRATION2_WITHOUT_FACTORY(Name,Parent,...) 
#include "..\BearGraphics\BearTemplate\BearGraphicsObjectsList.h"

DX12Stats::~DX12Stats()
{
}
