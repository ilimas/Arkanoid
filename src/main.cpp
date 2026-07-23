#include "Game.h"
#include <iostream>

// TODO: Gamepad support (WSL???)
// TODO: Difficility level

int main()
{
    try
    {
        Game g;
        return g.run();
    }
    catch (const std::exception &ex)
    {
        std::cerr << "Fatal: " << ex.what() << std::endl;
        return 1;
    }
}
