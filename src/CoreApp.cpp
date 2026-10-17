#include "CoreApp.h"

CoreApp::CoreApp()
        : OgreBites::ApplicationContext("OgreTestApp"), ufoController(nullptr),
            trayManager(nullptr), healthLabel(nullptr), displayedHealth(-1)
{
}

CoreApp::~CoreApp()
{
    if (trayManager) {
        removeInputListener(trayManager);
        delete trayManager;
        trayManager = nullptr;
        healthLabel = nullptr;
    }

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
    scnMgr->addRenderQueueListener(getOverlaySystem());

    Ogre::RTShader::ShaderGenerator *shadergen = Ogre::RTShader::ShaderGenerator::getSingletonPtr();
    shadergen->addSceneManager(scnMgr);

    // Create the camera, but not attached to the root scene node here
    Ogre::Camera *cam = scnMgr->createCamera("MainCam");
    cam->setNearClipDistance(5);
    cam->setAutoAspectRatio(true);

    Ogre::Viewport* viewport = getRenderWindow()->addViewport(cam);
    viewport->setBackgroundColour(Ogre::ColourValue(0.08f, 0.15f, 0.26f));

    // Sunset directional light
    Ogre::Light *light = scnMgr->createLight("MainLight");
    light->setType(Ogre::Light::LT_DIRECTIONAL);
    Ogre::SceneNode *lightNode = scnMgr->getRootSceneNode()->createChildSceneNode();
    lightNode->setDirection(Ogre::Vector3(-0.6f, -0.8f, 0.4f).normalisedCopy());
    lightNode->attachObject(light);
    light->setDiffuseColour(1.45f, 0.92f, 0.60f);
    light->setSpecularColour(1.65f, 1.05f, 0.72f);
    scnMgr->setAmbientLight(Ogre::ColourValue(0.28f, 0.24f, 0.32f));

    Ogre::ResourceGroupManager::getSingleton().addResourceLocation(
    "../Media", "FileSystem", "General");
    Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups();

    scnMgr->setSkyDome(true, "Examples/CloudySky", 5, 8, 500, true);
    scnMgr->setFog(Ogre::FOG_LINEAR, Ogre::ColourValue(0.08f, 0.15f, 0.26f), 0.0f, 220.0f, 620.0f);

    // Entity creation
    Ogre::Entity *ufoEntity = scnMgr->createEntity("UFO", "ufo.mesh");
    ufoEntity->setMaterialName("ufo.objMaterial.005");

    ufoEntity->setQueryFlags(PLAYER_MASK);
    
    // 1. Create the base translation node (Root)
    Ogre::SceneNode* playerRoot = scnMgr->getRootSceneNode()->createChildSceneNode("PlayerRoot");
    
    // 2. Create the visual node for the spinning animation (Child of Root)
    Ogre::SceneNode* visualNode = playerRoot->createChildSceneNode("UFOVisuals");
    visualNode->attachObject(ufoEntity);
    
    // Apply scaling to the visual node
    visualNode->setScale(10.0f, 10.0f, 10.0f); 


    // --- TEMPORARY TEST COW ---
    // Using the ogrehead as a placeholder for the cow 
    Ogre::Entity* testCow = scnMgr->createEntity("TestCow", "ogrehead.mesh");
    
    // Apply the cow mask so the tractor beam raycast can detect it
    testCow->setQueryFlags(COW_MASK); 
    
    Ogre::SceneNode* testCowNode = scnMgr->getRootSceneNode()->createChildSceneNode("TestCowNode");
    testCowNode->attachObject(testCow);
    
    // Position it slightly in front of and below the UFO starting position
    testCowNode->setPosition(0, -50, -50); 
    testCowNode->setScale(0.5f, 0.5f, 0.5f);
    // --------------------------


    // Initialize the controller with the split nodes and the camera
    ufoController = new UFOController(scnMgr, playerRoot, visualNode, cam);

    trayManager = new OgreBites::TrayManager("GameUI", getRenderWindow(), this);
    trayManager -> hideCursor();
    addInputListener(trayManager);
    displayedHealth = ufoController->getHealth();
    healthLabel = trayManager->createLabel(
        OgreBites::TL_TOPLEFT, "HealthLabel",
        "UFO HEALTH: " + std::to_string(displayedHealth), 220);
    trayManager->showTrays();
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

        int currentHealth = ufoController->getHealth();
        if (healthLabel && currentHealth != displayedHealth) {
            displayedHealth = currentHealth;
            healthLabel->setCaption("UFO HEALTH: " + std::to_string(displayedHealth));
        }
    }
}

bool CoreApp::mouseMoved(const OgreBites::MouseMotionEvent &evt)
{
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