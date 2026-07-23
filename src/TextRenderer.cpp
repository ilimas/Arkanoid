#include "TextRenderer.h"
#include "stb_truetype.h"
#include <algorithm>
#include <cmath>
#include <fstream>
#include <stdexcept>

namespace
{
std::vector<unsigned int> decodeUtf8(const std::string &s)
{
    std::vector<unsigned int> out;
    size_t i = 0;
    while (i < s.size())
    {
        unsigned char c = (unsigned char)s[i];
        unsigned int cp = 0;
        int extra = 0;
        if ((c & 0x80) == 0)
        {
            cp = c;
            extra = 0;
        }
        else if ((c & 0xE0) == 0xC0)
        {
            cp = c & 0x1Fu;
            extra = 1;
        }
        else if ((c & 0xF0) == 0xE0)
        {
            cp = c & 0x0Fu;
            extra = 2;
        }
        else if ((c & 0xF8) == 0xF0)
        {
            cp = c & 0x07u;
            extra = 3;
        }
        else
        {
            i++;
            continue; // invalid leading byte, skip
        }
        i++;
        bool valid = true;
        for (int k = 0; k < extra; k++)
        {
            if (i >= s.size() || (((unsigned char)s[i]) & 0xC0) != 0x80)
            {
                valid = false;
                break;
            }
            cp = (cp << 6) | (((unsigned char)s[i]) & 0x3Fu);
            i++;
        }
        if (valid)
            out.push_back(cp);
    }
    return out;
}
} // namespace

TextRenderer::TextRenderer(const std::string &ttfPath)
{
    std::ifstream in(ttfPath, std::ios::binary | std::ios::ate);
    if (!in.is_open())
        throw std::runtime_error("TextRenderer: cannot open font " + ttfPath);
    std::streamsize size = in.tellg();
    in.seekg(0, std::ios::beg);
    fontData_.resize((size_t)size);
    if (size > 0 && !in.read((char *)fontData_.data(), size))
        throw std::runtime_error("TextRenderer: cannot read font " + ttfPath);

    auto *info = new stbtt_fontinfo();
    int offset = stbtt_GetFontOffsetForIndex(fontData_.data(), 0);
    if (offset < 0 || !stbtt_InitFont(info, fontData_.data(), offset))
    {
        delete info;
        throw std::runtime_error("TextRenderer: stbtt_InitFont failed for " + ttfPath);
    }
    fontInfo_ = info;
}

TextRenderer::~TextRenderer() { delete static_cast<stbtt_fontinfo *>(fontInfo_); }

TextRenderer::Bitmap TextRenderer::render(const std::string &utf8, int fontSizePx) const
{
    Bitmap result;
    if (utf8.empty() || fontSizePx <= 0)
        return result;

    auto *info = static_cast<stbtt_fontinfo *>(fontInfo_);
    float scale = stbtt_ScaleForPixelHeight(info, (float)fontSizePx);
    int ascent, descent, lineGap;
    stbtt_GetFontVMetrics(info, &ascent, &descent, &lineGap);
    (void)lineGap;
    int baseline = (int)std::lround(ascent * scale);
    int lineHeight = (int)std::lround((ascent - descent) * scale);
    if (lineHeight <= 0)
        return result;

    std::vector<unsigned int> codepoints = decodeUtf8(utf8);
    if (codepoints.empty())
        return result;

    // First pass: total pen advance across the string gives the bitmap width.
    float penX = 0.0f;
    for (size_t i = 0; i < codepoints.size(); ++i)
    {
        int advanceWidth, leftSideBearing;
        stbtt_GetCodepointHMetrics(info, (int)codepoints[i], &advanceWidth, &leftSideBearing);
        penX += advanceWidth * scale;
        if (i + 1 < codepoints.size())
            penX += stbtt_GetCodepointKernAdvance(info, (int)codepoints[i], (int)codepoints[i + 1]) * scale;
    }
    int width = std::max(1, (int)std::lround(penX));

    result.w = width;
    result.h = lineHeight;
    result.rgba.assign((size_t)width * lineHeight * 4, 0);

    // Second pass: rasterize each glyph (white RGB, alpha = coverage) and
    // blit it into the shared buffer at its pen position.
    penX = 0.0f;
    for (size_t i = 0; i < codepoints.size(); ++i)
    {
        int cp = (int)codepoints[i];
        int advanceWidth, leftSideBearing;
        stbtt_GetCodepointHMetrics(info, cp, &advanceWidth, &leftSideBearing);

        int x0, y0, x1, y1;
        stbtt_GetCodepointBitmapBox(info, cp, scale, scale, &x0, &y0, &x1, &y1);
        int gw = x1 - x0, gh = y1 - y0;
        if (gw > 0 && gh > 0)
        {
            std::vector<unsigned char> glyph((size_t)gw * gh);
            stbtt_MakeCodepointBitmap(info, glyph.data(), gw, gh, gw, scale, scale, cp);

            int destX0 = (int)std::lround(penX) + x0;
            int destY0 = baseline + y0;
            for (int gy = 0; gy < gh; gy++)
            {
                int dy = destY0 + gy;
                if (dy < 0 || dy >= lineHeight)
                    continue;
                for (int gx = 0; gx < gw; gx++)
                {
                    int dx = destX0 + gx;
                    if (dx < 0 || dx >= width)
                        continue;
                    unsigned char a = glyph[(size_t)gy * gw + gx];
                    if (a == 0)
                        continue;
                    unsigned char *p = &result.rgba[((size_t)dy * width + dx) * 4];
                    p[0] = 255;
                    p[1] = 255;
                    p[2] = 255;
                    p[3] = a;
                }
            }
        }

        penX += advanceWidth * scale;
        if (i + 1 < codepoints.size())
            penX += stbtt_GetCodepointKernAdvance(info, cp, (int)codepoints[i + 1]) * scale;
    }

    return result;
}
