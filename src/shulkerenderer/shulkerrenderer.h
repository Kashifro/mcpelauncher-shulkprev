#pragma once
#include "ui/minecraftuirendercontext.h"
#include "hooks/minecraftuirendercontexthook.h"
#include "shulkerenderer/colors.h"
#include "ui/hashedstring.h"
#include "ui/nineslicehelper.h"
#include "ui/resourcelocation.h"
#include "item/itemstackbase.h"
#include "render/helper.h"
#include "util/scache.h"
#include "util/config.h"
#include "item/item.h"


class ShulkerRenderer {
public:

    static void render(
        MinecraftUIRenderContext* ctx,
        float x, float y,
        int index,
        char colorCode);
};
