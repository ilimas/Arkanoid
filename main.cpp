#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_TTF.h>
#include <SDL_mixer.h>
#include <stdio.h>
#include <math.h>
#include <windows.h>
#include <stdlib.h>
#include <time.h>
#include <vector>
#include "blocks.h"
#include "bblock.h"
#include "shar.h"
#include "image.h"
using namespace std;

int main(int argc, char* args[])
{
    int tstart=SDL_GetTicks();
    int n=0, gameover=0, begin=0, score=0;
    Mix_Music *music = NULL;
    SDL_Event event;
    int mx, my;
    double ti=1;
    srand(time(0));
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
    {
        printf("SDL_Init Error: %s\n", SDL_GetError());
        return 1;
    }
    if(TTF_Init()!=0) printf("Error ttf");
    SDL_Window *win = SDL_CreateWindow("Shalnoi", 20, 100, 640, 480, SDL_WINDOW_SHOWN);
    if (!win)
    {
        printf("SDL_CreateWindow Error: %s\n", SDL_GetError());
        return 1;
    }
    SDL_Renderer *ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!ren)
    {
        printf("SDL_CreateRenderer Error: %s\n", SDL_GetError());
        return 1;
    }
    blocks bl;
    shar sh;
    b_block bblock(ren);
    image D(ren);
    if(Mix_OpenAudio(22050, MIX_DEFAULT_FORMAT, 2, 4096)==-1) return 2;
    music = Mix_LoadMUS("shrek.wav");
    if(!music) return 1;
    SDL_RenderClear(ren);
    if(Mix_PlayMusic(music, -1)==-1) return 1;
    while(1)
    {
        while(1)
        {
            D.draw_menu(n);
            SDL_RenderPresent(ren);
            SDL_GetMouseState(&mx, &my);
            if(mx>=450 && mx<=600 && my<=130 && my>=100) n=0;
            else if(mx>=450 && mx<=600 && my<=170 && my>=140) n=1;
            if(SDL_PollEvent(&event))
            {
                if (event.type == SDL_MOUSEBUTTONDOWN  &&  event.button.button==SDL_BUTTON_LEFT && mx<=600 && mx>=450 && my>=100 && my<=170)
                {
                    if(n==0)
                    {
                        bl.load_level();
                        break;
                    }
                    else if(n==1)
                    {
                        Mix_FreeMusic(music);
                        SDL_DestroyRenderer(ren);
                        SDL_DestroyWindow(win);
                        Mix_CloseAudio();
                        TTF_Quit();
                        SDL_Quit();
                        return 3;
                    }
                }
            }
        }
        while(!gameover)
        {
            if(!begin)
            {
                D.draw_background();
                bblock.Draw_Block();
                bl.Draw_Blocks(ren);
                sh.Draw(ren);
                D.draw_begin();
                SDL_RenderPresent(ren);
                while(1)
                {
                    if(SDL_PollEvent(&event))
                    {
                        if(event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) break;
                    }
                }
                begin=1;
            }
            SDL_GetMouseState(&mx, NULL);
            if(mx>=0 && mx<=540) bblock.setpos(mx);
            if(SDL_PollEvent(&event))
            {
                if(event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE)
                {
                    D.draw_pause();
                    SDL_RenderPresent(ren);
                    while(1)
                    {
                        if(SDL_PollEvent(&event))
                        {
                            if(event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_RETURN)
                            {
                                gameover=1;
                                break;
                            }
                            else if(event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) break;
                        }
                    }
                }
            }
            if(begin)
            {
                //SDL_ShowCursor(SDL_DISABLE);
                D.draw_background();
                if(sh.rety()>=470)
                {
                    D.draw_end();
                    SDL_RenderPresent(ren);
                    while(1)
                    {
                        if(SDL_PollEvent(&event) && event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_RETURN)
                        {
                            gameover=1;
                            break;
                        }
                    }
                }
                if(sh.retx()<=630 && sh.retx()>=10 && sh.rety()>=10)
                {
                    sh.next_step();
                    for(int i=0; i<bl.bsize(); i++)
                    {
                        if((sh.rety()-10-sh.retalf()*sh.retgy()<=bl.rety(i)+15 &&  sh.rety()-sh.retalf()*sh.retgy()>=bl.rety(i)+15 || sh.rety()+10-sh.retalf()*sh.retgy()>=bl.rety(i) &&  sh.rety()-sh.retalf()*sh.retgy()<=bl.rety(i)) && sh.retx()-10<=bl.retx(i)+50 && sh.retx()+10>=bl.retx(i))
                        {
                            sh.setgy();
                            bl.minus(i);
                            if(bl.retvar(i)==0)
                            {
                                bl.del(i);
                                score++;
                            }
                        }
                        else if((sh.retx()+10+2*sh.retgx()*sh.retfx()>=bl.retx(i) &&  sh.retx()+2*sh.retgx()*sh.retfx()<=bl.retx(i) || sh.retx()-10+2*sh.retgx()*sh.retfx()<=bl.retx(i)+50 &&  sh.retx()+2*sh.retgx()*sh.retfx()>=bl.retx(i)+50) && sh.rety()-10<=bl.rety(i)+15 && sh.rety()+10>=bl.rety(i))
                        {
                            sh.setgx();
                            bl.minus(i);
                            if(bl.retvar(i)==0)
                            {
                                bl.del(i);
                                score++;
                            }
                        }
                    }
                    if(sh.retx()>=bblock.retx() && sh.retx()<=bblock.retx()+100 && sh.rety()+13>=bblock.rety())
                    {
                        if(sh.retx()<=bblock.retx()+52 && sh.retx()>=bblock.retx()+48)
                        {
                            sh.setfx(0);
                        }
                        else
                        {
                            sh.setfx(1);
                            if(sh.retx()<=bblock.retx()+50)
                            {
                                if(sh.retgx()>0) sh.setgx();
                                if(sh.retx()<=bblock.retx()+10) sh.setalf(2);
                                else if(sh.retx()>bblock.retx()+10 && sh.retx()<=bblock.retx()+30) sh.setalf(3);
                                else sh.setalf(4);
                            }
                            else
                            {
                                if(sh.retgx()<0) sh.setgx();
                                if(sh.retx()>=bblock.retx()+90) sh.setalf(2);
                                else if(sh.retx()<bblock.retx()+90 && sh.retx()>=bblock.retx()+70) sh.setalf(3);
                                else sh.setalf(4);
                            }
                        }
                        sh.setgy();
                    }
                }
                else
                {
                    if((sh.retx()>=630 && sh.rety()<=10) || (sh.retx()<=10 && sh.rety()<=10) || (sh.retx()<=10 && sh.rety()>=470) || (sh.retx()>=630 && sh.rety()>=470))
                    {
                        sh.setgx();
                        sh.setgy();
                    }
                    else if(sh.retx()<=10 || sh.retx()>=630) sh.setgx();
                    else if(sh.rety()<=10 || sh.rety()>=470) sh.setgy();
                    sh.next_step();
                }
                bblock.Draw_Block();
                bl.Draw_Blocks(ren);
                sh.Draw(ren);
                SDL_RenderPresent(ren);
            }
            if(bl.bsize()==0)
            {
                ti=SDL_GetTicks()-tstart;
                ti/=1000;
                D.level_cleared(ti/score);
                SDL_RenderPresent(ren);
                while(1)
                {
                    if(SDL_PollEvent(&event) && event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_RETURN)
                    {
                        gameover=1;
                        break;
                    }
                }
            }
        }
        SDL_ShowCursor(SDL_ENABLE);
        begin=0;
        gameover=0;
        score=0;
        tstart=SDL_GetTicks();
        bl.setmain();
        sh.setmain();
        bblock.setmain();
    }
    Mix_FreeMusic(music);
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    Mix_CloseAudio();
    TTF_Quit();
    SDL_Quit();
    return 0;
}


