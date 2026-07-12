#include "FrontendManager.h"
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_render.h>
#include <SDL_rwops.h>
#include <SDL_stdinc.h>
#include <SDL_ttf.h>
#include <iostream>
#include <stdexcept>

FrontendManager::FrontendManager(SDL_Renderer *ren)
{
    if (TTF_Init() != 0)
    {
        std::cerr << "TTF_Init error\n";
    }
    fnt = TTF_OpenFont(SNACKS_DIR "/ttf/Casper_B.ttf", 30);
    if (!fnt)
        throw std::runtime_error("Unable to load font");
    render = ren;
    background = SDL_LoadBMP(SNACKS_DIR "/img/back.bmp");
    tbackground = SDL_CreateTextureFromSurface(ren, background);
    SDL_FreeSurface(background);
    menu = SDL_LoadBMP(SNACKS_DIR "/img/menu.bmp");
    tmenu = SDL_CreateTextureFromSurface(ren, menu);
    SDL_FreeSurface(menu);
}

FrontendManager::~FrontendManager()
{
    TTF_CloseFont(fnt);
    fnt = nullptr;
    TTF_Quit();
    SDL_DestroyTexture(tmenu);
    SDL_DestroyTexture(tbackground);
}

void FrontendManager::draw_text(const std::string &utf8, int x, int y, SDL_Color color, int fontSize)
{
    TTF_SetFontSize(fnt, fontSize);
    SDL_Surface *surf = TTF_RenderUTF8_Blended(fnt, utf8.c_str(), color);
    if (!surf)
        return;
    SDL_Texture *tex = SDL_CreateTextureFromSurface(render, surf);
    SDL_Rect r{x, y, surf->w, surf->h};
    SDL_RenderCopy(render, tex, NULL, &r);
    SDL_FreeSurface(surf);
    SDL_DestroyTexture(tex);
    TTF_SetFontSize(fnt, 30);
}

void FrontendManager::draw_welcome_text()
{
    SDL_Rect r;
    SDL_Color color;
    color.r = 255;
    color.g = 185;
    color.b = 15;
    TTF_SetFontSize(fnt, 20);
    text = TTF_RenderUTF8_Blended(fnt, "Нажмите левую кнопку мыши, чтобы начать", color);
    TTF_SetFontSize(fnt, 30);
    ftext = SDL_CreateTextureFromSurface(render, text);
    r.x = 80;
    r.y = 250;
    r.h = text->h;
    r.w = text->w;
    SDL_RenderCopy(render, ftext, NULL, &r);
    SDL_FreeSurface(text);
    SDL_DestroyTexture(ftext);
}

void FrontendManager::draw_pause()
{
    SDL_Rect r;
    SDL_Color color;
    color.r = 255;
    color.g = 185;
    color.b = 15;
    text = TTF_RenderUTF8_Blended(fnt, "Игра на паузе", color);
    ftext = SDL_CreateTextureFromSurface(render, text);
    r.x = 230;
    r.y = 150;
    r.h = text->h;
    r.w = text->w;
    SDL_RenderCopy(render, ftext, NULL, &r);
    text = TTF_RenderUTF8_Blended(fnt, "Esc - продолжить", color);
    ftext = SDL_CreateTextureFromSurface(render, text);
    r.x = 230;
    r.y = 180;
    r.h = text->h;
    r.w = text->w;
    SDL_RenderCopy(render, ftext, NULL, &r);
    text = TTF_RenderUTF8_Blended(fnt, "Enter - выход", color);
    ftext = SDL_CreateTextureFromSurface(render, text);
    r.x = 230;
    r.y = 210;
    r.h = text->h;
    r.w = text->w;
    SDL_RenderCopy(render, ftext, NULL, &r);
    SDL_FreeSurface(text);
    SDL_DestroyTexture(ftext);
}

void FrontendManager::draw_background() { SDL_RenderCopy(render, tbackground, NULL, nullptr); }

void FrontendManager::level_cleared(double a)
{
    SDL_Rect r;
    SDL_Color color;
    char buf[200];
    color.r = 255;
    color.g = 185;
    color.b = 15;
    TTF_SetFontSize(fnt, 20);
    sprintf(buf, "Уровень пройден, ваш ранк: %.2lf, нажмите Enter для возврата в меню", a);
    text = TTF_RenderUTF8_Blended(fnt, buf, color);
    TTF_SetFontSize(fnt, 30);
    ftext = SDL_CreateTextureFromSurface(render, text);
    r.x = 30;
    r.y = 130;
    r.h = text->h;
    r.w = text->w;
    SDL_RenderCopy(render, ftext, NULL, &r);
    SDL_FreeSurface(text);
    SDL_DestroyTexture(ftext);
}

void FrontendManager::draw_end()
{
    SDL_Rect r;
    SDL_Color color;
    color.r = 255;
    color.g = 185;
    color.b = 15;
    TTF_SetFontSize(fnt, 20);
    text = TTF_RenderUTF8_Blended(fnt, "Игра окончена, нажмите Enter для возврата в меню", color);
    TTF_SetFontSize(fnt, 30);
    ftext = SDL_CreateTextureFromSurface(render, text);
    r.x = 80;
    r.y = 130;
    r.h = text->h;
    r.w = text->w;
    SDL_RenderCopy(render, ftext, NULL, &r);
    SDL_FreeSurface(text);
    SDL_DestroyTexture(ftext);
}

void FrontendManager::draw_menu(int menu)
{
    SDL_RenderCopy(render, tmenu, NULL, NULL);
    static const char *labels[3] = {"Новая Игра", "Выход", "Таблица игроков"};
    static const int ys[3] = {100, 140, 180};
    SDL_Color selectedColor{255, 185, 15, 255};
    SDL_Color normalColor{255, 52, 179, 255};
    for (int i = 0; i < 3; i++)
        draw_text(labels[i], 450, ys[i], (i == menu) ? selectedColor : normalColor, 30);
}

void FrontendManager::draw_name_input(const std::string &currentName)
{
    SDL_Color prompt{255, 185, 15, 255};
    draw_text("Введите имя игрока:", 200, 200, prompt, 24);
    std::string display = currentName + "_";
    draw_text(display, 200, 250, prompt, 30);
    SDL_Color hint{255, 52, 179, 255};
    draw_text("Enter - начать игру, Esc - в меню", 200, 320, hint, 18);
}

void FrontendManager::draw_leaderboard(const std::vector<PlayerDatabase::Entry> &entries)
{
    SDL_Color title{255, 185, 15, 255};
    draw_text("Таблица игроков (всего сбито блоков)", 90, 50, title, 24);
    SDL_Color row{255, 255, 255, 255};
    int y = 110;
    int rank = 1;
    for (const auto &e : entries)
    {
        if (rank > 10)
            break;
        std::string line = std::to_string(rank) + ". " + e.name + " - " + std::to_string(e.totalBlocks);
        draw_text(line, 120, y, row, 20);
        y += 30;
        rank++;
    }
    if (entries.empty())
        draw_text("Пока нет результатов", 120, y, row, 20);
    SDL_Color hint{255, 52, 179, 255};
    draw_text("Enter / Esc / клик - назад в меню", 120, 560, hint, 18);
}
