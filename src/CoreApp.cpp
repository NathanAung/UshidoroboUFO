#include "CoreApp.h"

CoreApp::CoreApp() : OgreBites::ApplicationContext("OgreTestApp"), headNode(nullptr), mainCamera(nullptr)
{
}

CoreApp::~CoreApp()
{
    
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

    // Ogre head entity
    Ogre::Entity *ogreHead = scnMgr->createEntity("Head", "ogrehead.mesh");
    headNode = scnMgr->getRootSceneNode()->createChildSceneNode();
    headNode->attachObject(ogreHead);
}

bool CoreApp::keyPressed(const OgreBites::KeyboardEvent &evt)
{
    OgreBites::Keycode key = evt.keysym.sym;

    if (key == OgreBites::SDLK_ESCAPE)
    {
        getRoot()->queueEndRendering();
    }

    // keep track of the player's intended direction
    if (key == 'w')
        headNode->translate(0, 1, 0);
    else if (key == 'a')
        headNode->translate(-1, 0, 0);
    else if (key == 's')
        headNode->translate(0, -1, 0);
    else if (key == 'd')
        headNode->translate(1, 0, 0);

    else if (key == OgreBites::SDLK_SPACE)
    {
    }

    return true;
}

bool CoreApp::keyReleased(const OgreBites::KeyboardEvent &evt)
{
    return true;
}

void CoreApp::frameRendered(const Ogre::FrameEvent &evt)
{
    headNode->yaw(Ogre::Degree(10.0f * evt.timeSinceLastFrame));
}

bool CoreApp::mouseMoved(const OgreBites::MouseMotionEvent &evt)
{
    // update camera goal based on mouse movement
    if (mainCamera)
    {
        mainCamera->yaw(Ogre::Degree(0.05f * evt.xrel));
        mainCamera->pitch(Ogre::Degree(0.05f * evt.yrel));
    }
    return true;
}

bool CoreApp::mouseWheelRolled(const OgreBites::MouseWheelEvent &evt)
{
    // update camera goal based on mouse movement
    if (headNode)
    {
        headNode->scale(evt.y * 2, evt.y * 2, evt.y * 2);
    }
    return true;
}

bool CoreApp::mousePressed(const OgreBites::MouseButtonEvent &evt)
{
    return true;
}