#include <SDL.h>
#include <SDL_TTF.h>
#include <stdio.h>
#include <windows.h>
#include <stdlib.h>
#include "image.h"
image::image(SDL_Renderer *ren)
{
    render=ren;
    background=SDL_LoadBMP("back.bmp");
    tbackground=SDL_CreateTextureFromSurface(ren, background);
    SDL_FreeSurface(background);
    menu=SDL_LoadBMP("menu.bmp");
    tmenu=SDL_CreateTextureFromSurface(ren, menu);
    SDL_FreeSurface(menu);
}
image::~image()
{
    SDL_DestroyTexture(tmenu);
    SDL_DestroyTexture(tbackground);
}
void image::draw_begin()
{
    TTF_Font *fnt=NULL;
    SDL_Rect r;
    SDL_Color color;
    fnt=TTF_OpenFont("Casper_B.ttf", 25);
    if (!fnt) printf("Unable to load font \n");
    color.r=255;
    color.g=185;
    color.b=15;
    text=TTF_RenderUTF8_Blended(fnt, "Нажмите левую кнопку мыши, чтобы начать", color);
    ftext=SDL_CreateTextureFromSurface(render, text);
    r.x=80;
    r.y=250;
    r.h=text->h;
    r.w=text->w;
    SDL_RenderCopy(render, ftext, NULL, &r);
    SDL_FreeSurface(text);
    SDL_DestroyTexture(ftext);
    TTF_CloseFont(fnt);
}
void image::draw_pause()
{
    TTF_Font *fnt=NULL;
    SDL_Rect r;
    SDL_Color color;
    fnt=TTF_OpenFont("Casper_B.ttf", 30);
    if (!fnt) printf("Unable to load font \n");
    color.r=255;
    color.g=185;
    color.b=15;
    text=TTF_RenderUTF8_Blended(fnt, "Игра на паузе", color);
    ftext=SDL_CreateTextureFromSurface(render, text);
    r.x=230;
    r.y=150;
    r.h=text->h;
    r.w=text->w;
    SDL_RenderCopy(render, ftext, NULL, &r);
    text=TTF_RenderUTF8_Blended(fnt, "Esc - продолжить", color);
    ftext=SDL_CreateTextureFromSurface(render, text);
    r.x=230;
    r.y=180;
    r.h=text->h;
    r.w=text->w;
    SDL_RenderCopy(render, ftext, NULL, &r);
    text=TTF_RenderUTF8_Blended(fnt, "Enter - выход", color);
    ftext=SDL_CreateTextureFromSurface(render, text);
    r.x=230;
    r.y=210;
    r.h=text->h;
    r.w=text->w;
    SDL_RenderCopy(render, ftext, NULL, &r);
    SDL_FreeSurface(text);
    SDL_DestroyTexture(ftext);
    TTF_CloseFont(fnt);
}
void image::draw_background()
{
    SDL_RenderCopy(render, tbackground, NULL, NULL);
}
void image::level_cleared(double a)
{
    TTF_Font *fnt=NULL;
    SDL_Rect r;
    SDL_Color color;
    char buf[200];
    fnt=TTF_OpenFont("Casper_B.ttf", 20);
    if (!fnt) printf("Unable to load font \n");
    color.r=255;
    color.g=185;
    color.b=15;
    sprintf(buf, "Уровень пройден, ваш ранк: %.2lf, нажмите Enter для возврата в меню", a);
    text=TTF_RenderUTF8_Blended(fnt, buf, color);
    ftext=SDL_CreateTextureFromSurface(render, text);
    r.x=30;
    r.y=130;
    r.h=text->h;
    r.w=text->w;
    SDL_RenderCopy(render, ftext, NULL, &r);
    SDL_FreeSurface(text);
    SDL_DestroyTexture(ftext);
    TTF_CloseFont(fnt);
}
void image::draw_end()
{
    TTF_Font *fnt=NULL;
    SDL_Rect r;
    SDL_Color color;
    fnt=TTF_OpenFont("Casper_B.ttf", 20);
    if (!fnt) printf("Unable to load font \n");
    color.r=255;
    color.g=185;
    color.b=15;
    text=TTF_RenderUTF8_Blended(fnt, "Игра окончена, нажмите Enter для возврата в меню", color);
    ftext=SDL_CreateTextureFromSurface(render, text);
    r.x=80;
    r.y=130;
    r.h=text->h;
    r.w=text->w;
    SDL_RenderCopy(render, ftext, NULL, &r);
    SDL_FreeSurface(text);
    SDL_DestroyTexture(ftext);
    TTF_CloseFont(fnt);
}
void image::draw_menu(int menu)
{
    TTF_Font *fnt=NULL;
    SDL_Rect r;
    SDL_Color color;
    fnt=TTF_OpenFont("Casper_B.ttf", 30);
    if (!fnt) printf("Unable to load font \n");
    SDL_RenderCopy(render, tmenu, NULL, NULL);
    if(menu==0)
    {
        color.r=255;
        color.g=185;
        color.b=15;
        text=TTF_RenderUTF8_Blended(fnt, "Новая Игра", color);
        ftext=SDL_CreateTextureFromSurface(render, text);
        r.x=450;
        r.y=100;
        r.h=text->h;
        r.w=text->w;
        SDL_RenderCopy(render, ftext, NULL, &r);
        color.r=255;
        color.g=52;
        color.b=179;
        text=TTF_RenderUTF8_Blended(fnt, "Выход", color);
        ftext=SDL_CreateTextureFromSurface(render, text);
        r.x=450;
        r.y=140;
        r.h=text->h;
        r.w=text->w;
        SDL_RenderCopy(render, ftext, NULL, &r);
        SDL_FreeSurface(text);
    }
    else if(menu==1)
    {
        color.r=255;
        color.g=52;
        color.b=179;
        text=TTF_RenderUTF8_Blended(fnt, "Новая Игра", color);
        ftext=SDL_CreateTextureFromSurface(render, text);
        r.x=450;
        r.y=100;
        r.h=text->h;
        r.w=text->w;
        SDL_RenderCopy(render, ftext, NULL, &r);
        color.r=255;
        color.g=185;
        color.b=15;
        text=TTF_RenderUTF8_Blended(fnt, "Выход", color);
        ftext=SDL_CreateTextureFromSurface(render, text);
        r.x=450;
        r.y=140;
        r.h=text->h;
        r.w=text->w;
        SDL_RenderCopy(render, ftext, NULL, &r);
        SDL_FreeSurface(text);
    }
    SDL_DestroyTexture(ftext);
    TTF_CloseFont(fnt);
}
