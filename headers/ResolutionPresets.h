#ifndef RESOLUTION_PRESETS_H
#define RESOLUTION_PRESETS_H

// Selectable window/output resolutions for the Settings screen. The game's
// logical canvas (UILayout::ScreenW x ScreenH) never changes - GLRenderer
// letterboxes it to fit whatever window size is picked here, up to real 4K.
struct ResolutionPreset
{
    int w;
    int h;
    const char *label;
};

constexpr int kResolutionPresetCount = 4;
constexpr ResolutionPreset kResolutionPresets[kResolutionPresetCount] = {
    {1280, 720, "1280 x 720"},
    {1920, 1080, "1920 x 1080 (Full HD)"},
    {2560, 1440, "2560 x 1440 (2K)"},
    {3840, 2160, "3840 x 2160 (4K)"},
};

#endif
