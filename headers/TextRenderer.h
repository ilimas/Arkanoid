#ifndef TEXT_RENDERER_H
#define TEXT_RENDERER_H

#include <string>
#include <vector>

// Loads a TTF font once (via stb_truetype) and rasterizes UTF-8 strings into
// tightly-cropped RGBA8 pixel buffers (white glyphs, real alpha) - replaces
// SDL_ttf. The caller uploads the returned buffer as a texture and tints it
// at draw time (see FrontendManager::TextCache).
class TextRenderer
{
  public:
    explicit TextRenderer(const std::string &ttfPath);
    ~TextRenderer();
    TextRenderer(const TextRenderer &) = delete;
    TextRenderer &operator=(const TextRenderer &) = delete;

    struct Bitmap
    {
        std::vector<unsigned char> rgba;
        int w = 0;
        int h = 0;
    };

    // Renders `utf8` at `fontSizePx` (pixel line height, matches the old
    // TTF_SetFontSize semantics closely enough for this game's UI).
    Bitmap render(const std::string &utf8, int fontSizePx) const;

  private:
    std::vector<unsigned char> fontData_;
    void *fontInfo_; // stbtt_fontinfo*, kept opaque so callers don't need stb_truetype.h
};

#endif
