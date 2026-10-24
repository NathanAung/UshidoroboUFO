#include "CoreApp.h"

int main(int argc, char** argv)
{
    bool restart = true;

    while (restart)
    {
        CoreApp app;
        app.initApp();
        app.getRoot()->startRendering();

        restart = app.wantsRestart();
        app.closeApp();
    }

    return 0;
}