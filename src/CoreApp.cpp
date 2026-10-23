
#include "CoreApp.h"

CoreApp::CoreApp()
    : OgreBites::ApplicationContext("OgreTestApp"),
      ufoController(nullptr),
      spawnManager(nullptr),
      trayManager(nullptr),
      mAudioManager(nullptr),
      mParticleManager(nullptr),
      healthLabel(nullptr),
      displayedHealth(-1)
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
    if (trayManager) {
        removeInputListener(trayManager);
        delete trayManager;
        trayManager = nullptr;
        healthLabel = nullptr;
    }


    scnMgr->addRenderQueueListener(getOverlaySystem());

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

    // Generate Terrain Mesh
    TerrainController terrainController;
    int width = 1000;
    int depth = 1000;
    float scale = 1.0f;

    Ogre::Entity *baseEntity = terrainController.GenerateTerrainMesh(scnMgr, width, depth, scale);

    // Calculate the exact physical size of one chunk
    float physicalWidth = (width - 1) * scale;
    float physicalDepth = (depth - 1) * scale;

    for (int xGrid = -1; xGrid <= 1; ++xGrid)
    {
        for (int zGrid = -1; zGrid <= 1; ++zGrid)
        {
            // Create unique names for each chunk's entity and node
            std::string nodeName = "TerrainNode_" + std::to_string(xGrid) + "_" + std::to_string(zGrid);
            std::string entityName = "TerrainEntity_" + std::to_string(xGrid) + "_" + std::to_string(zGrid);

            Ogre::Entity *terrainEntity;

            // Reuse the already created mesh for the neighbors to save memory
            if (xGrid == 0 && zGrid == 0)
            {
                terrainEntity = baseEntity;
            }
            else
            {
                terrainEntity = scnMgr->createEntity(entityName, "TerrainMesh");
            }

            // Create the scene node and attach the entity
            terrainEntity->setQueryFlags(ENVIRONMENT_MASK);
            Ogre::SceneNode *terrainNode = scnMgr->getRootSceneNode()->createChildSceneNode(nodeName);
            terrainNode->attachObject(terrainEntity);

            // Position each chunk based on its grid coordinate offset (1000 units wide/deep)
            float worldX = (xGrid * physicalWidth - (width / 2.0f));
            float worldZ = (zGrid * physicalWidth - (depth / 2.0f));

            terrainNode->setPosition(worldX, -50.0f, worldZ);
        }
    }

    ufoEntity->setQueryFlags(PLAYER_MASK);

    // 1. Create the base translation node (Root)
    Ogre::SceneNode *playerRoot = scnMgr->getRootSceneNode()->createChildSceneNode("PlayerRoot");

    // 2. Create the visual node for the spinning animation (Child of Root)
    Ogre::SceneNode *visualNode = playerRoot->createChildSceneNode("UFOVisuals");
    visualNode->attachObject(ufoEntity);

    // Apply scaling to the visual node
    visualNode->setScale(10.0f, 10.0f, 10.0f);

    // --- TEMPORARY TEST COW ---
    // Using the ogrehead as a placeholder for the cow
    Ogre::Entity *testCow = scnMgr->createEntity("TestCow", "ogrehead.mesh");

    // Apply the cow mask so the tractor beam raycast can detect it
    testCow->setQueryFlags(COW_MASK);

    Ogre::SceneNode *testCowNode = scnMgr->getRootSceneNode()->createChildSceneNode("TestCowNode");
    testCowNode->attachObject(testCow);

    // Position it slightly in front of and below the UFO starting position
    testCowNode->setPosition(0, -50, -50);
    testCowNode->setScale(0.5f, 0.5f, 0.5f);
    // --------------------------

    // Initialize the Particle Manager first
    mParticleManager = new ParticleManager(scnMgr);

    // Initialize the spawn manager
    spawnManager = new SpawnManager(scnMgr);

    spawnManager->setParticleManager(mParticleManager);
    
    spawnManager->setUFONode(playerRoot);

    // Include AudioManager.h and initialize it in setup()
    mAudioManager = new AudioManager();

    // Start your background track immediately
    mAudioManager->playBGM("bgm.mp3");

    // Create UFOController ONCE
    ufoController = new UFOController(
        scnMgr,
        playerRoot,
        visualNode,
        cam,
        mParticleManager, 
        mAudioManager
    );

    ufoController->setSpawnManager(spawnManager);

    spawnManager->setUFOController(ufoController);

    // Spawn some test objects
    spawnManager->spawnRandom(SpawnType::Cow, 50);
    spawnManager->spawnRandom(SpawnType::Tank, 30);

    trayManager = new OgreBites::TrayManager("GameUI", getRenderWindow(), this);
    trayManager->hideCursor();
    addInputListener(trayManager);
    displayedHealth = ufoController->getHealth();
    healthLabel = trayManager->createLabel(
        OgreBites::TL_TOPLEFT, "HealthLabel",
        "UFO HEALTH: " + std::to_string(displayedHealth), 220);
    trayManager->showTrays();
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
    {
        ufoController->update(evt);

        int currentHealth = ufoController->getHealth();
        if (healthLabel && currentHealth != displayedHealth)
        {
            displayedHealth = currentHealth;

            healthLabel->setCaption(
                "UFO HEALTH: " +
                std::to_string(displayedHealth)
            );
        }
    }

    if (mParticleManager)
    {
        mParticleManager->update(evt.timeSinceLastFrame);
    }

    if (spawnManager)
    {
        spawnManager->update(evt.timeSinceLastFrame);
    }
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
