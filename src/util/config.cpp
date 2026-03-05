#include "config.h"

#include <fstream>

#include <properties/property.h>
#include <properties/property_list.h>

int spPreviewKey = 'H';
bool spChangingPreviewKey = false;
float spTintIntensity = 1.0f;

namespace {
properties::property_list configList('=');
properties::property<int> previewKey(configList, "previewKey", 'H');
properties::property<int> tintIntensity(configList, "tintIntensity", 100);
} // namespace

int SP_clampPercent(int value) {
    if (value < 0)
        return 0;
    if (value > 200)
        return 200;
    return value;
}

std::string SP_getConfigPath() {
    return "../shulkerpreview.conf";
}

void SP_loadConfig() {
    std::ifstream file(SP_getConfigPath());
    if (file)
        configList.load(file);

    spPreviewKey = previewKey.get();
    if (spPreviewKey <= 0)
        spPreviewKey = 'H';

    const int intensityPercent = SP_clampPercent(tintIntensity.get());
    spTintIntensity = static_cast<float>(intensityPercent) / 100.0f;
}

void SP_saveConfig() {
    previewKey.set(spPreviewKey);
    tintIntensity.set(SP_clampPercent(static_cast<int>(spTintIntensity * 100.0f + 0.5f)));

    std::ofstream file(SP_getConfigPath());
    if (file)
        configList.save(file);
}
