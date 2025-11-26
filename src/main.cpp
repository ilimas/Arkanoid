#include "Game.h"
#include <iostream>

// TODO: Gamepad support (WSL???)
// TODO: Black hole logic
// TODO: Difficility level
// TODO: Hidden areas

int main(int argc, char *argv[])
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
