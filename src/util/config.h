#pragma once

#include <string>

extern int spPreviewKey;
extern bool spChangingPreviewKey;
extern float spTintIntensity;
extern bool spAnchorAboveTooltip;

std::string SP_getConfigPath();
void SP_loadConfig();
void SP_saveConfig();
int SP_clampPercent(int value);
