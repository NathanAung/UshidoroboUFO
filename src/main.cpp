#include "CoreApp.h"

int main(int argc, char** argv) {
    CoreApp app;
    app.initApp();
    app.getRoot()->startRendering();
    app.closeApp();
    return 0;
}
 