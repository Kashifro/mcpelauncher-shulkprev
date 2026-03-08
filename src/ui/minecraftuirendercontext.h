#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include <glm/glm.hpp>

#pragma pack(push, 4)
struct RectangleArea {
    float _x0;
    float _x1;
    float _y0;
    float _y1;
};
#pragma pack(pop)

#pragma pack(push, 4)
struct ImageInfo {
    glm::vec2 mPosition;
    glm::vec2 mSize;
    glm::vec2 mUV;
    glm::vec2 mUVSize;
};
#pragma pack(pop)

struct NinesliceInfo {
    ImageInfo mTopLeft;
    ImageInfo mTopRight;
    ImageInfo mBottomLeft;
    ImageInfo mBottomRight;
    glm::vec2 mUVScale;
    std::vector<ImageInfo> mLeft;
    std::vector<ImageInfo> mTop;
    std::vector<ImageInfo> mRight;
    std::vector<ImageInfo> mBottom;
    std::vector<ImageInfo> mMiddle;
};

class ResourceLocation;

namespace mce {
struct Color {
    float r;
    float g;
    float b;
    float a;
};

// Opaque client texture handle
struct ClientTexture {
    std::byte _storage[24]{};
};
} // namespace mce

// Only the first field matters here; TexturePtr::getClientTexture() reads this member.
struct BedrockTextureData {
    mce::ClientTexture mClientTexture;
};

namespace mce {
class TexturePtr {
public:
    std::shared_ptr<const ::BedrockTextureData> mClientTexture;
    std::shared_ptr<::ResourceLocation> mResourceLocation;

public:
    const ClientTexture& getClientTexture() const {
        static const ClientTexture kEmpty{};
        return mClientTexture ? mClientTexture->mClientTexture : kEmpty;
    }
};
} // namespace mce

static_assert(sizeof(mce::ClientTexture) == 24);
static_assert(sizeof(mce::TexturePtr) == 32);

namespace ui {
enum class TextAlignment : uint8_t {
    Left,
    Right,
    Center
};
} // namespace ui

#pragma pack(push, 4)
struct TextMeasureData {
    float fontSize;
    float linePadding;
    bool renderShadow;
    bool showColorSymbol;
    bool hideHyphen;
};
#pragma pack(pop)

#pragma pack(push, 4)
struct CaretMeasureData {
    int position;
    bool shouldRender;
};
#pragma pack(pop)

struct ComponentRenderBatch;
struct MinecraftUIMeasureStrategy;
class Font;
class UITextureInfoPtr;
class HashedString;
class CustomRenderComponent;
class ClientInstance;
class ScreenContext;
class UIRepository;
class UIScene;
namespace Core {
class Path;
}
//minFontHandle
class FontHandle {
public:
    std::byte mFontRepository[24]; // NonOwnerPointer<FontRepository>
    std::byte mDefaultFont[16];    // std::shared_ptr<Font>
    uint64_t  mFontId;
    bool      mIsDummyHandle;
    std::byte _pad[7];
};

static_assert(sizeof(FontHandle) == 56);

class MinecraftUIRenderContext {
public:
    ClientInstance* mClient;                  // 0x08
    ScreenContext* mScreenContext;            // 0x10
    std::byte mMeasureStrategy[0x20];         // 0x18
    float mTextAlpha;                         // 0x38
    std::byte _pad3C[4];                      // 0x3C
    UIRepository* mUIRepository;              // 0x40
    void* mUIService1;                        // 0x48
    void* mUIService2;                        // 0x50
    std::byte _pad58[0x28];                   // 0x58
    mce::TexturePtr** mTextureGroups;         // 0x80
    std::byte _pad88[0x48];                   // 0x88
    FontHandle* mDebugFontHandle;             // 0xD0
    const UIScene* mCurrentScene;             // 0x120

public:
    virtual ~MinecraftUIRenderContext();
    virtual float getLineLength(Font&, const std::string&, float, bool);
    virtual float getTextAlpha();
    virtual void setTextAlpha(float);

    virtual void drawDebugText(
        const RectangleArea&,
        const std::string&,
        const mce::Color&,
        ui::TextAlignment,
        float,
        const TextMeasureData&,
        const CaretMeasureData&
    );

    virtual void drawText(
        Font&,
        const RectangleArea&,
        const std::string&,
        const mce::Color&,
        ui::TextAlignment,
        float,
        const TextMeasureData&,
        const CaretMeasureData&
    );

    virtual void flushText(float, std::optional<float>);

    virtual void drawImage(
        const mce::ClientTexture&,
        const glm::vec2&,
        const glm::vec2&,
        const glm::vec2&,
        const glm::vec2&,
        bool
    );

    virtual void drawNineslice(const mce::ClientTexture&, const NinesliceInfo&);
    virtual void flushImages(const mce::Color&, float, const HashedString&);
    virtual void beginSharedMeshBatch(ComponentRenderBatch&);
    virtual void endSharedMeshBatch(ComponentRenderBatch&);
    virtual void reserveSharedMeshBatch(std::uint64_t);
    virtual std::uint64_t getSharedMeshBatchVertexCount() const;
    virtual void drawRectangle(const RectangleArea&, const mce::Color&, float, int);
    virtual void fillRectangle(const RectangleArea&, const mce::Color&, float);
    virtual void increaseStencilRef();
    virtual void decreaseStencilRef();
    virtual void resetStencilRef();
    virtual void fillRectangleStencil(const RectangleArea&);
    virtual void enableScissorTest(const RectangleArea&);
    virtual void disableScissorTest();
    virtual void setClippingRectangle(const RectangleArea&);
    virtual void setFullClippingRectangle();
    virtual void saveCurrentClippingRectangle();
    virtual void restoreSavedClippingRectangle();
    virtual RectangleArea getFullClippingRectangle() const;
    virtual bool updateCustom(CustomRenderComponent*);
    virtual void renderCustom(CustomRenderComponent*, int, RectangleArea&);
    virtual void cleanup();
    virtual void removePersistentMeshes();
    virtual mce::TexturePtr getTexture(const ResourceLocation&, bool) const;
    virtual mce::TexturePtr getZippedTexture(const Core::Path&, const ResourceLocation&, bool) const;
    virtual bool unloadTexture(const ResourceLocation&);
    virtual UITextureInfoPtr getUITextureInfo(const ResourceLocation&, bool) const;
    virtual void touchTexture(const ResourceLocation&);
    virtual MinecraftUIMeasureStrategy* getMeasureStrategy();
    virtual void snapImageSizeToGrid(glm::vec2&) const;
    virtual void snapImagePositionToGrid(glm::vec2&) const;
    virtual void notifyImageEstimate(std::uint64_t);
};

