#include "App.h"

#include <windows.h>

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
    App app;
    if (app.createWindow() != 0)
    {
        return -1;
    }
    app.initD2D();

    app.run();

    return 0;
}