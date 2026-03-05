#include "shulkerrenderer.h"

#include <cstdio>
#include <xmmintrin.h>

#include "util/config.h"

namespace {
constexpr int kColumns = 9;
constexpr int kRows = 3;
constexpr float kSlotStride = 18.0f;
constexpr float kSlotDrawSize = 17.5f;
constexpr float kPanelPadding = 6.0f;
constexpr float kItemDrawSize = 16.0f;
constexpr float kItemInset = (kSlotStride - kItemDrawSize) * 0.5f;
constexpr float kCountTextHeight = 6.0f;

const HashedString kFlushMaterial("ui_flush");
const NinesliceHelper kPanelNineSlice(16.0f, 16.0f, 4.0f, 4.0f);
const mce::Color kWhite{1.0f, 1.0f, 1.0f, 1.0f};

float clamp01(float v){
    if(v < 0) return 0;
    if(v > 1) return 1;
    return v;
}

mce::Color applyTintIntensity(const mce::Color& base){
    float i = spTintIntensity;
    return {
        clamp01(base.r * i),
        clamp01(base.g * i),
        clamp01(base.b * i),
        base.a
    };
}

struct CachedUiTextures{
    bool loaded = false;
    mce::TexturePtr panel;
    mce::TexturePtr slot;
};

inline bool hasTexture(const mce::TexturePtr& tex){
    return (bool)tex.mClientTexture;
}

template <typename Fn>
void forEachSlot(float ox,float oy,Fn&& fn){
    for(int r=0;r<kRows;r++){
        for(int c=0;c<kColumns;c++){
            int idx=r*kColumns+c;
            float x=ox+c*kSlotStride;
            float y=oy+r*kSlotStride;
            fn(idx,x,y);
        }
    }
}

ItemStackBase* getRenderableStack(ShulkerSlotCache& cache){
    if(!cache.valid) return nullptr;

    ItemStackBase* stack = asISB(cache.isb);
    if(!stack) return nullptr;
    if(!stack->mItem.get()) return nullptr;

    return stack;
}

bool hasEnchantedSlots(int cacheIndex){
    for(int i=0;i<SHULKER_SLOT_COUNT;i++){
        const ShulkerSlotCache& sc=ShulkerCache[cacheIndex][i];
        if(sc.valid && sc.enchanted)
            return true;
    }
    return false;
}

void* getMinecraftGame(void* clientInstance){
    if(!clientInstance) return nullptr;

    auto** vt = *(void***)clientInstance;

    if(vt && vt[kClientGetMinecraftGameVfIndex]){
        auto fn = (void*(*)(void*))vt[kClientGetMinecraftGameVfIndex];
        if(void* r = fn(clientInstance))
            return r;
    }

    return *(void**)((char*)clientInstance + kClientMinecraftGameOffset);
}

void destroyBaseActorRenderContext(void* barc){
    if(!barc) return;

    auto** vt=*(void***)barc;
    if(!vt || !vt[0]) return;

    auto dtor=(void(*)(void*))vt[0];
    dtor(barc);
}

void drawDurabilityBar(
    MinecraftUIRenderContext& ctx,
    ItemStackBase* stack,
    float slotX,
    float slotY)
{
    Item* item = stack->mItem.get();
    if(!item) return;

    short maxDamage=item->getMaxDamage();
    if(maxDamage<=0) return;

    short damage=ItemStackBase_getDamageValue(stack);
    if(damage<=0) return;

    float ratio=(float)(maxDamage-damage)/(float)maxDamage;

    float bx=slotX+2.0f;
    float by=slotY+13.0f;

    RectangleArea bg{
        bx,
        bx+13.0f,
        by,
        by+2.0f
    };

    ctx.fillRectangle(bg,mce::Color{0,0,0,1},1.0f);

    float barWidth=13.0f*ratio;

    RectangleArea bar{
        bx,
        bx+barWidth,
        by,
        by+1.0f
    };

    ctx.fillRectangle(bar,mce::Color{1.0f-ratio,ratio,0,1},1.0f);
}

CachedUiTextures& getUiTextures(MinecraftUIRenderContext& ctx){
    static CachedUiTextures tex;

    if(!tex.loaded){
        tex.panel=ctx.getTexture(
            ResourceLocation("textures/ui/dialog_background_opaque",ResourceFileSystem::UserPackage),
            false);

        tex.slot=ctx.getTexture(
            ResourceLocation("textures/ui/item_cell",ResourceFileSystem::UserPackage),
            false);

        tex.loaded=true;
    }

    return tex;
}

void drawPanelTexture(
    MinecraftUIRenderContext& ctx,
    const CachedUiTextures& tex,
    const RectangleArea& rect)
{
    if(!hasTexture(tex.panel)) return;
    kPanelNineSlice.draw(ctx,rect,tex.panel.getClientTexture());
}

void drawSlotTexture(
    MinecraftUIRenderContext& ctx,
    const CachedUiTextures& tex,
    const RectangleArea& rect)
{
    if(!hasTexture(tex.slot)) return;

    glm::vec2 pos{rect._x0,rect._y0};

    glm::vec2 size{
        rect._x1-rect._x0,
        rect._y1-rect._y0
    };

    ctx.drawImage(
        tex.slot.getClientTexture(),
        pos,size,
        {0,0},{1,1},
        false
    );
}

void drawStackCountText(
    Font& font,
    float slotX,
    float slotY,
    const char* text,
    const TextMeasureData& measure,
    const CaretMeasureData& caret)
{
    float w = ActiveUIContext->getLineLength(font,text,measure.fontSize,false);

    float ax=slotX+kSlotDrawSize-0.5f;
    float ay=slotY+kSlotDrawSize-1.5f;

    RectangleArea r{
        ax-w,
        ax,
        ay-kCountTextHeight,
        ay
    };

    ActiveUIContext->drawText(
        font,r,text,
        mce::Color{1,1,1,1},
        ui::TextAlignment::Right,
        1.0f,
        measure,
        caret
    );
}

void drawSlotIcons(
    MinecraftUIRenderContext& ctx,
    const CachedUiTextures& tex,
    int cacheIndex,
    float ox,
    float oy)
{
    if(!BaseActorRenderContext_ctor || !ItemRenderer_renderGuiItemNew)
        return;

    if(!ctx.mClient || !ctx.mScreenContext)
        return;

    void* clientInstance=ctx.mClient;
    void* minecraftGame=getMinecraftGame(clientInstance);
    if(!minecraftGame) return;

    alignas(16) std::byte barcStorage[kBarcStorageSize]{};
    void* barc = barcStorage;

    BaseActorRenderContext_ctor(
        barc,
        ctx.mScreenContext,
        clientInstance,
        minecraftGame
    );

    void* itemRenderer=
        *(void**)((std::byte*)barc + kBarcItemRendererOffset);

    if(!itemRenderer){
        destroyBaseActorRenderContext(barc);
        return;
    }

    // icon pass
    forEachSlot(ox,oy,[&](int slot,float x,float y){

        ShulkerSlotCache& sc=ShulkerCache[cacheIndex][slot];

        ItemStackBase* stack=getRenderableStack(sc);
        if(!stack) return;

        float drawX=x+kItemInset;
        float drawY=y+kItemInset;

        __m128 posX=_mm_set1_ps(drawX);
        __m128 posY=_mm_set1_ps(drawY);

        ItemRenderer_renderGuiItemNew(
            itemRenderer,
            barc,
            stack,
            0,
            0,
            0,
            posX,posY,
            1.0f,1.0f,1.0f
        );
    });

    // glint pass
    if(hasEnchantedSlots(cacheIndex)){
        forEachSlot(ox,oy,[&](int slot,float x,float y){

            ShulkerSlotCache& sc=ShulkerCache[cacheIndex][slot];
            if(!sc.enchanted) return;

            ItemStackBase* stack=getRenderableStack(sc);
            if(!stack) return;

            float drawX=x+kItemInset;
            float drawY=y+kItemInset;

            __m128 posX=_mm_set1_ps(drawX);
            __m128 posY=_mm_set1_ps(drawY);

            ItemRenderer_renderGuiItemNew(
                itemRenderer,
                barc,
                stack,
                0,
                1,
                0,
                posX,posY,
                1.0f,1.0f,1.0f
            );
        });
    }

    // durability bars
    forEachSlot(ox,oy,[&](int slot,float x,float y){

        ShulkerSlotCache& sc=ShulkerCache[cacheIndex][slot];
        if(!sc.valid) return;

        ItemStackBase* stack=getRenderableStack(sc);
        if(!stack) return;

        drawDurabilityBar(ctx,stack,x,y);
    });

    destroyBaseActorRenderContext(barc);

    ctx.flushImages(kWhite,1.0f,kFlushMaterial);
}

} // namespace



void ShulkerRenderer::render(
    MinecraftUIRenderContext* ctx,
    float x,
    float y,
    int index,
    char colorCode)
{
    if(!ctx || !ActiveUIContext || !ActiveUIFont)
        return;

    const mce::Color tint=
        applyTintIntensity(getShulkerTint(colorCode));

    const CachedUiTextures& tex=getUiTextures(*ctx);

    RectangleArea panel{
        x,
        x+kColumns*kSlotStride+kPanelPadding*2,
        y,
        y+kRows*kSlotStride+kPanelPadding*2
    };

    drawPanelTexture(*ctx,tex,panel);

    ctx->flushImages(tint,1.0f,kFlushMaterial);

    float ox=x+kPanelPadding;
    float oy=y+kPanelPadding;

    Font& font=*ActiveUIFont;

    TextMeasureData measure{};
    measure.fontSize=1.0f;

    CaretMeasureData caret{};

    forEachSlot(ox,oy,[&](int,float sx,float sy){

        RectangleArea rect{
            sx,
            sx+kSlotDrawSize,
            sy,
            sy+kSlotDrawSize
        };

        drawSlotTexture(*ctx,tex,rect);
    });

    ctx->flushImages(tint,1.0f,kFlushMaterial);

    drawSlotIcons(*ctx,tex,index,ox,oy);

    forEachSlot(ox,oy,[&](int slot,float sx,float sy){

        ShulkerSlotCache& sc=ShulkerCache[index][slot];

        if(!sc.valid || sc.count<=1)
            return;

        char txt[8];
        std::snprintf(txt,sizeof(txt),"%u",sc.count);

        drawStackCountText(font,sx,sy,txt,measure,caret);
    });

    ctx->flushText(0.0f,std::nullopt);
}