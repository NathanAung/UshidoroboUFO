
#include "CoreApp.h"

CoreApp::CoreApp()
    : OgreBites::ApplicationContext("OgreTestApp"),
      ufoController(nullptr),
      spawnManager(nullptr),
      mParticleManager(nullptr)
{
}

CoreApp::~CoreApp()
{
    delete ufoController;
    ufoController = nullptr;

    delete spawnManager;
    spawnManager = nullptr;

    delete mParticleManager;
    mParticleManager = nullptr;
}

void CoreApp::setup()
{
    OgreBites::ApplicationContext::setup();
    addInputListener(this);

    Ogre::Root* root = getRoot();
    Ogre::SceneManager* scnMgr = root->createSceneManager();

    Ogre::RTShader::ShaderGenerator* shadergen =
        Ogre::RTShader::ShaderGenerator::getSingletonPtr();
    shadergen->addSceneManager(scnMgr);

    // Camera
    Ogre::Camera* cam = scnMgr->createCamera("MainCam");
    cam->setNearClipDistance(5);
    cam->setAutoAspectRatio(true);

    Ogre::Viewport* viewport = getRenderWindow()->addViewport(cam);
    viewport->setBackgroundColour(
        Ogre::ColourValue(0.08f, 0.15f, 0.26f)
    );

    // Lighting
    Ogre::Light* light = scnMgr->createLight("MainLight");
    light->setType(Ogre::Light::LT_DIRECTIONAL);

    Ogre::SceneNode* lightNode =
        scnMgr->getRootSceneNode()->createChildSceneNode();

    lightNode->setDirection(
        Ogre::Vector3(-0.6f, -0.8f, 0.4f).normalisedCopy()
    );
    lightNode->attachObject(light);

    light->setDiffuseColour(1.15f, 0.75f, 0.52f);
    light->setSpecularColour(1.25f, 0.85f, 0.62f);

    scnMgr->setAmbientLight(
        Ogre::ColourValue(0.22f, 0.18f, 0.26f)
    );

    // Resources and environment
    Ogre::ResourceGroupManager::getSingleton().addResourceLocation(
        "../Media", "FileSystem", "General"
    );

    Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups();

    scnMgr->setSkyDome(true, "Examples/CloudySky", 5, 8, 500, true);

    scnMgr->setFog(
        Ogre::FOG_LINEAR,
        Ogre::ColourValue(0.08f, 0.15f, 0.26f),
        0.0f, 220.0f, 620.0f
    );

    // UFO entity
    Ogre::Entity* ufoEntity =
        scnMgr->createEntity("UFO", "ufo.mesh");

    ufoEntity->setMaterialName("ufo.objMaterial.005");
    ufoEntity->setQueryFlags(PLAYER_MASK);

    // Terrain
    TerrainController terrainController;

    Ogre::Entity* terrainEntity =
        terrainController.GenerateTerrainMesh(
            scnMgr, 1000, 1000, 1.0f
        );

    // IMPORTANT: Terrain must not be captured as a cow
    terrainEntity->setQueryFlags(ENVIRONMENT_MASK);

    Ogre::SceneNode* terrainNode =
        scnMgr->getRootSceneNode()->createChildSceneNode("TerrainNode");

    terrainNode->attachObject(terrainEntity);
    terrainNode->setPosition(0, -50, 0);

    // Player root node
    Ogre::SceneNode* playerRoot =
        scnMgr->getRootSceneNode()->createChildSceneNode("PlayerRoot");

    // UFO visual node
    Ogre::SceneNode* visualNode =
        playerRoot->createChildSceneNode("UFOVisuals");

    visualNode->attachObject(ufoEntity);
    visualNode->setScale(10.0f, 10.0f, 10.0f);

    // Initialize managers in the correct order
    mParticleManager = new ParticleManager(scnMgr);

    // SpawnManager MUST exist before passing it to UFOController
    spawnManager = new SpawnManager(scnMgr);
    spawnManager->setUFONode(playerRoot);

    ufoController = new UFOController(
        scnMgr,
        playerRoot,
        visualNode,
        cam,
        mParticleManager
    );

    ufoController->setSpawnManager(spawnManager);

    // Spawn test cows
    spawnManager->spawnRandom(SpawnType::Cow, 50);
}

bool CoreApp::keyPressed(const OgreBites::KeyboardEvent& evt)
{
    OgreBites::Keycode key = evt.keysym.sym;

    if (key == OgreBites::SDLK_ESCAPE)
    {
        getRoot()->queueEndRendering();
        return true;
    }

    if (ufoController)
        ufoController->injectKeyDown(evt);

    return true;
}

bool CoreApp::keyReleased(const OgreBites::KeyboardEvent& evt)
{
    if (ufoController)
        ufoController->injectKeyUp(evt);

    return true;
}

void CoreApp::frameRendered(const Ogre::FrameEvent& evt)
{
    if (ufoController)
        ufoController->update(evt);

    if (mParticleManager)
        mParticleManager->update(evt.timeSinceLastFrame);

    if (spawnManager)
        spawnManager->update(evt.timeSinceLastFrame);
}

bool CoreApp::mouseMoved(const OgreBites::MouseMotionEvent& evt)
{
    return true;
}

bool CoreApp::mouseWheelRolled(const OgreBites::MouseWheelEvent& evt)
{
    return true;
}

bool CoreApp::mousePressed(const OgreBites::MouseButtonEvent& evt)
{
    return true;
}
