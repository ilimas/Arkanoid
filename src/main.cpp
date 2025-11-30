#include "Game.h"
#include <SDL.h>
#include <SDL_version.h>
#include <iostream>

// TODO: Gamepad support (WSL???)
// TODO: Black hole logic
// TODO: Difficility level
// TODO: Hidden areas

int main(int argc, char *argv[])
{
    try
    {
        SDL_version compiled;
        SDL_version linked;

        SDL_VERSION(&compiled);

        SDL_GetVersion(&linked);

        std::cout << "Compiled with SDL version: " << int(compiled.major) << "."
                  << int(compiled.minor) << "." << int(compiled.patch) << std::endl;

        std::cout << "Linked SDL version: " << int(linked.major) << "." << int(linked.minor) << "."
                  << int(linked.patch) << std::endl;
        Game g;
        return g.run();
    }
    catch (const std::exception &ex)
    {
        std::cerr << "Fatal: " << ex.what() << std::endl;
        return 1;
    }
}
