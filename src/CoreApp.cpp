#include "CoreApp.h"

CoreApp::CoreApp() : OgreBites::ApplicationContext("OgreTestApp"), mainCamera(nullptr), ufoController(nullptr)
{
}

CoreApp::~CoreApp()
{
    // Clean up the dynamically allocated controller
    if (ufoController) {
        delete ufoController;
        ufoController = nullptr;
    }
}

void CoreApp::setup()
{
    OgreBites::ApplicationContext::setup();
    addInputListener(this);

    Ogre::Root *root = getRoot();
    Ogre::SceneManager *scnMgr = root->createSceneManager();

    Ogre::RTShader::ShaderGenerator *shadergen = Ogre::RTShader::ShaderGenerator::getSingletonPtr();
    shadergen->addSceneManager(scnMgr);

    // Camera
    Ogre::Camera *cam = scnMgr->createCamera("MainCam");
    cam->setNearClipDistance(5);
    cam->setAutoAspectRatio(true);
    mainCamera = scnMgr->getRootSceneNode()->createChildSceneNode();
    mainCamera->attachObject(cam);
    mainCamera->setPosition(0, 0, 200);
    getRenderWindow()->addViewport(cam);

    // Light
    Ogre::Light *light = scnMgr->createLight("MainLight");
    Ogre::SceneNode *lightNode = scnMgr->getRootSceneNode()->createChildSceneNode();
    lightNode->setPosition(20, 80, 50);
    lightNode->attachObject(light);
    scnMgr->setSkyBox(true, "Examples/CloudyNoonSkyBox", 5000, true);

    // Ogre automatically uses the Assimp codec when it detects the .obj extension
    Ogre::Entity *ufoEntity = scnMgr->createEntity("UFO", "toyUFO.obj");
    Ogre::SceneNode* ufoNode = scnMgr->getRootSceneNode()->createChildSceneNode();
    ufoNode->attachObject(ufoEntity);

    // Pass the node to the controller
    ufoController = new UFOController(scnMgr, ufoNode);
}

bool CoreApp::keyPressed(const OgreBites::KeyboardEvent &evt)
{
    OgreBites::Keycode key = evt.keysym.sym;

    // Handle global application hotkeys first
    if (key == OgreBites::SDLK_ESCAPE)
    {
        getRoot()->queueEndRendering();
        return true;
    }

    // Delegate player input to the controller
    if (ufoController) {
        ufoController->injectKeyDown(evt);
    }

    return true;
}

bool CoreApp::keyReleased(const OgreBites::KeyboardEvent &evt)
{
    if (ufoController) {
        ufoController->injectKeyUp(evt);
    }
    return true;
}

void CoreApp::frameRendered(const Ogre::FrameEvent &evt)
{
    // Delegate frame updates to the controller
    if (ufoController) {
        ufoController->update(evt);
    }
}

bool CoreApp::mouseMoved(const OgreBites::MouseMotionEvent &evt)
{
    if (mainCamera)
    {
        mainCamera->yaw(Ogre::Degree(0.05f * evt.xrel));
        mainCamera->pitch(Ogre::Degree(0.05f * evt.yrel));
    }
    return true;
}

bool CoreApp::mouseWheelRolled(const OgreBites::MouseWheelEvent &evt)
{
    // Removed the headNode scaling here to fully decouple the player from CoreApp
    return true;
}

bool CoreApp::mousePressed(const OgreBites::MouseButtonEvent &evt)
{
    return true;
}