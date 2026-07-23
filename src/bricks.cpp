#include "Bricks.h"
#include "Ball.h"
#include "ProceduralTextures.h"
#include "Utils.h"
#include <algorithm>
#include <cstdlib>

BlockField::BlockField(GLRenderer &gl_)
{
    gl = &gl_;
    for (int i = 0; i < 26; i++)
        for (int j = 0; j < 16; j++)
            map[i][j] = 0;

    texGreen = ProceduralTextures::makeBrickTexture(*gl, 80, 24, Color{96, 214, 110, 255});
    texOrange = ProceduralTextures::makeBrickTexture(*gl, 80, 24, Color{255, 150, 60, 255});
    texStone = ProceduralTextures::makeBrickTexture(*gl, 80, 24, Color{128, 132, 145, 255});
}

BlockField::~BlockField()
{
    gl->destroyTexture(texGreen);
    gl->destroyTexture(texOrange);
    gl->destroyTexture(texStone);
}

int BlockField::rety(int i) { return a[i].r.y; }
int BlockField::retx(int i) { return a[i].r.x; }
int BlockField::remaining() { return (int)a.size(); }

void BlockField::del(int i) { a.erase(a.begin() + i); }
void BlockField::setmain() { a.clear(); }

int BlockField::destructibleCount()
{
    int count = 0;
    for (auto &b : a)
        if (b.var > 0) count++;
    return count;
}

void BlockField::load_level(int levelNum)
{
    (void)levelNum;
    a.clear();
    for (int i = 0; i < 26; i++)
        for (int j = 0; j < 16; j++)
            map[i][j] = 0;

    if (bagIndex >= levelBag.size())
    {
        int prevLast = levelBag.empty() ? -1 : levelBag.back();
        levelBag = {0, 1, 2, 3, 4, 5};
        do
        {
            std::shuffle(levelBag.begin(), levelBag.end(), rng);
        } while (levelBag.front() == prevLast);
        bagIndex = 0;
    }
    int pattern = levelBag[bagIndex++];

    switch (pattern)
    {
    case 0: // Stripes
        for (int j = 0; j < 16; j++)
        {
            map[0][j] = 1; map[2][j] = 1;
            map[4][j] = 2; map[6][j] = 2;
        }
        break;

    case 1: // Pyramid (wide at top, narrow at bottom)
        for (int i = 0; i < 8; i++)
            for (int j = i; j < 16 - i; j++)
                map[i][j] = (i < 4) ? 1 : 2;
        break;

    case 2: // Diamond
        for (int i = 0; i < 8; i++)
        {
            int half = (i < 4) ? i : 7 - i;
            int lo = 7 - half, hi = 8 + half;
            for (int j = lo; j <= hi; j++)
                map[i][j] = (j == lo || j == hi || i == 0 || i == 7) ? 2 : 1;
        }
        break;

    case 3: // Vertical stone pillars with normal bricks between
        for (int i = 0; i < 8; i++)
            for (int j = 0; j < 16; j++)
            {
                if (j % 4 == 0)
                    map[i][j] = -1; // stone at cols 0, 4, 8, 12
                else
                    map[i][j] = ((i + j / 4) % 2 == 0) ? 1 : 2;
            }
        break;

    case 4: // Checkerboard
        for (int i = 0; i < 7; i++)
            for (int j = 0; j < 16; j++)
                map[i][j] = ((i + j) % 2 == 0) ? 1 : 2;
        break;

    case 5: // Hard random with stones
        for (int i = 0; i < 12; i++)
            for (int j = 0; j < 16; j++)
            {
                int r = std::rand() % 10;
                if (r == 0)      map[i][j] = -1; // 10% stone
                else if (r <= 3) map[i][j] = 2;  // 30% double-hit
                else if (r <= 6) map[i][j] = 1;  // 30% single-hit
                // else 30% empty
            }
        break;
    }

    temp.r.h = 24;
    temp.r.w = 80;
    for (int i = 0; i < 26; i++)
        for (int j = 0; j < 16; j++)
        {
            if (map[i][j] == 0) continue;
            temp.r.y = i * 24 + 48; // 48px top margin keeps blocks away from wall-bounce zone
            temp.r.x = j * 80 + 80; // 80px left margin centers 16 cols in 1440px screen
            temp.var = map[i][j];   // -1=stone, 1=single-hit, 2=double-hit
            a.push_back(temp);
        }

    startingSize = (uint32_t)a.size();
    destructibleStartingSize = (uint32_t)destructibleCount();
}

int BlockField::retvar(int i) { return a[i].var; }

void BlockField::minus(int i) { a[i].var--; }

void BlockField::draw()
{
    for (int i = 0; i < (int)a.size(); i++)
    {
        const GLRenderer::Texture &tex = (a[i].var < 0) ? texStone : (a[i].var == 1 ? texGreen : texOrange);
        gl->drawTexture(tex, a[i].r);
    }
}

HitSide SingleBlock::detectHitSide(const Vec2 &ballPos, double radius)
{
    Vec2 closest_point = closestPoint(this->r, ballPos);
    float dx = ballPos.x - closest_point.x;
    float dy = ballPos.y - closest_point.y;
    if (std::fabs(dx) > std::fabs(dy))
        return (dx > 0) ? HitSide::HIT_LEFT : HitSide::HIT_RIGHT;
    else
        return (dy > 0) ? HitSide::HIT_TOP : HitSide::HIT_BOTTOM;
    return HitSide::HIT_NONE;
}

Vec2 SingleBlock::reflectBall(Ball &ball)
{
    Vec2 v = ball.get_destination();
    auto side = detectHitSide(ball.get_position(), ball.radius);
    switch (side)
    {
    case HitSide::HIT_LEFT:
    case HitSide::HIT_RIGHT:
        v.x = -v.x; break;
    case HitSide::HIT_TOP:
    case HitSide::HIT_BOTTOM:
        v.y = -v.y; break;
    default:
        break;
    }
    return v;
}
