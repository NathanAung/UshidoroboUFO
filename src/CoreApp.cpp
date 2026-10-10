#include "CoreApp.h"

CoreApp::CoreApp() : OgreBites::ApplicationContext("OgreTestApp"), ufoController(nullptr)
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

    // Create the camera, but not attached to the root scene node here
    Ogre::Camera *cam = scnMgr->createCamera("MainCam");
    cam->setNearClipDistance(5);
    cam->setAutoAspectRatio(true);
    getRenderWindow()->addViewport(cam);

    // Light
    Ogre::Light *light = scnMgr->createLight("MainLight");
    Ogre::SceneNode *lightNode = scnMgr->getRootSceneNode()->createChildSceneNode();
    lightNode->setPosition(20, 80, 50);
    lightNode->attachObject(light);
    scnMgr->setSkyBox(true, "Examples/CloudyNoonSkyBox", 5000, true);

    // Entity creation
    Ogre::Entity *ufoEntity = scnMgr->createEntity("UFO", "ufo.mesh");

    // Generate Terrain Mesh
    TerrainController terrainController;
    Ogre::Entity *terrainEntity = terrainController.GenerateTerrainMesh(scnMgr, 1000, 1000, 1.0f);
    
    Ogre::SceneNode *terrainNode = scnMgr->getRootSceneNode()->createChildSceneNode("TerrainNode");
    terrainNode->attachObject(terrainEntity);
    terrainNode->setPosition(0, -50, 0); // Adjust the Y position to lower the terrain

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