
#include "CoreApp.h"
#include <cmath>
#include <OgreCompositorInstance.h>
#include <OgreCompositorManager.h>

namespace
{
    std::string formatTimer(int totalSeconds)
    {
        int minutes = totalSeconds / 60;
        int seconds = totalSeconds % 60;
        return (minutes < 10 ? "0" : "") + std::to_string(minutes) + ":" +
               (seconds < 10 ? "0" : "") + std::to_string(seconds);
    }
}

CoreApp::CoreApp()
    : OgreBites::ApplicationContext("OgreTestApp"),
      ufoController(nullptr),
      spawnManager(nullptr),
      trayManager(nullptr),
      mAudioManager(nullptr),
      mParticleManager(nullptr),
      healthLabel(nullptr),
      cowsLabel(nullptr),
      timerLabel(nullptr),
      displayedHealth(-1),
      displayedCows(-1),
      displayedTimerSeconds(-1)
{
}

CoreApp::~CoreApp()
{
    releaseOwnedObjects();
}

void CoreApp::shutdown()
{
    releaseOwnedObjects();
    OgreBites::ApplicationContext::shutdown();
}

void CoreApp::releaseOwnedObjects()
{
    if (trayManager)
    {
        removeInputListener(trayManager);
        delete trayManager;
        trayManager = nullptr;
    }
    healthLabel = nullptr;
    cowsLabel = nullptr;
    timerLabel = nullptr;

    delete ufoController;
    ufoController = nullptr;

    delete spawnManager;
    spawnManager = nullptr;

    delete mParticleManager;
    mParticleManager = nullptr;

    delete mAudioManager;
    mAudioManager = nullptr;
}

void CoreApp::setup()
{
    OgreBites::ApplicationContext::setup();
    addInputListener(this);

    Ogre::Root *root = getRoot();
    Ogre::SceneManager *scnMgr = root->createSceneManager();
    if (trayManager)
    {
        removeInputListener(trayManager);
        delete trayManager;
        trayManager = nullptr;
        healthLabel = nullptr;
        cowsLabel = nullptr;
        timerLabel = nullptr;
    }

    scnMgr->addRenderQueueListener(getOverlaySystem());

    Ogre::RTShader::ShaderGenerator *shadergen =
        Ogre::RTShader::ShaderGenerator::getSingletonPtr();
    shadergen->addSceneManager(scnMgr);

    // Camera
    Ogre::Camera *cam = scnMgr->createCamera("MainCam");
    cam->setNearClipDistance(5);
    cam->setAutoAspectRatio(true);

    Ogre::Viewport *viewport = getRenderWindow()->addViewport(cam);
    viewport->setBackgroundColour(
        Ogre::ColourValue(0.018f, 0.032f, 0.075f));

    scnMgr->setAmbientLight(
        Ogre::ColourValue(0.07f, 0.09f, 0.16f));
    scnMgr->setShadowTechnique(Ogre::SHADOWTYPE_TEXTURE_MODULATIVE);
    scnMgr->setShadowTextureSize(2048);
    scnMgr->setShadowColour(Ogre::ColourValue(0.04f, 0.05f, 0.09f));
    scnMgr->setShadowFarDistance(700.0f);

    Ogre::Light *moonLight = scnMgr->createLight("MoonLight");
    moonLight->setType(Ogre::Light::LT_DIRECTIONAL);
    moonLight->setDiffuseColour(0.10f, 0.14f, 0.25f);
    moonLight->setSpecularColour(0.06f, 0.09f, 0.18f);
    Ogre::SceneNode *moonLightNode =
        scnMgr->getRootSceneNode()->createChildSceneNode();
    moonLightNode->setDirection(
        Ogre::Vector3(-0.45f, -1.0f, 0.35f).normalisedCopy());
    moonLightNode->attachObject(moonLight);

    // Resources and environment

    Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups();
    Ogre::CompositorInstance *vignette =
        Ogre::CompositorManager::getSingleton().addCompositor(
            viewport, "UFO/HorrorVignette");
    if (!vignette)
    {
        OGRE_EXCEPT(
            Ogre::Exception::ERR_ITEM_NOT_FOUND,
            "Could not load the UFO horror vignette compositor.",
            "CoreApp::setup");
    }
    vignette->setEnabled(true);

    scnMgr->setFog(
        Ogre::FOG_LINEAR,
        Ogre::ColourValue(0.018f, 0.032f, 0.075f),
        0.0f, 1100.0f, 2600.0f);

    // UFO entity
    Ogre::Entity *ufoEntity =
        scnMgr->createEntity("UFO", "ufo.mesh");

    ufoEntity->setMaterialName("ufo.objMaterial.005");
    for (unsigned short i = 0; i < ufoEntity->getNumSubEntities(); ++i)
    {
        Ogre::Pass *ufoPass = ufoEntity->getSubEntity(i)
                                  ->getMaterial()
                                  ->getTechnique(0)
                                  ->getPass(0);
        ufoPass->setAmbient(0.025f, 0.04f, 0.08f);
        ufoPass->setSelfIllumination(0.12f, 0.24f, 0.42f);
    }

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
        mAudioManager);

    ufoController->setSpawnManager(spawnManager);

    spawnManager->setUFOController(ufoController);

    // Spawn some test objects
    spawnManager->spawnRandom(SpawnType::Cow, 50);
    spawnManager->spawnRandom(SpawnType::Tank, 30);

    trayManager = new OgreBites::TrayManager("GameUI", getRenderWindow(), this);
    trayManager->hideCursor();
    addInputListener(trayManager);
    displayedHealth = ufoController->getHealth();
    displayedCows = ufoController->getCowsAbducted();
    displayedTimerSeconds = static_cast<int>(
        std::ceil(ufoController->getGameTimer()));
    healthLabel = trayManager->createLabel(
        OgreBites::TL_TOPLEFT, "HealthLabel",
        "UFO HEALTH: " + std::to_string(displayedHealth), 220);
    cowsLabel = trayManager->createLabel(
        OgreBites::TL_TOPLEFT, "CowsLabel",
        "COWS ABDUCTED: " + std::to_string(displayedCows), 220);
    timerLabel = trayManager->createLabel(
        OgreBites::TL_TOPLEFT, "TimerLabel",
        "TIME LEFT: " + formatTimer(displayedTimerSeconds), 220);
    trayManager->showTrays();
}

bool CoreApp::keyPressed(const OgreBites::KeyboardEvent &evt)
{
    if (gameEnded)
        return true;
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

bool CoreApp::keyReleased(const OgreBites::KeyboardEvent &evt)
{
    if (gameEnded)
        return true;
    if (ufoController)
        ufoController->injectKeyUp(evt);

    return true;
}

void CoreApp::frameRendered(const Ogre::FrameEvent &evt)
{

    if (gameEnded)
        return;

    if (ufoController &&
        (ufoController->getHealth() <= 0 ||
         ufoController->getGameTimer() <= 0.0f))
    {
        trayManager->moveWidgetToTray(healthLabel, OgreBites::TL_CENTER);
        trayManager->moveWidgetToTray(cowsLabel, OgreBites::TL_CENTER);
        trayManager->moveWidgetToTray(timerLabel, OgreBites::TL_CENTER);

        gameEnded = true;

        healthLabel->setCaption("GAME OVER");
        cowsLabel->setCaption(
            "FINAL SCORE: " +
            std::to_string(ufoController->getCowsAbducted()));
        timerLabel->setCaption("Press ESC to quit");

        return;
    }

    if (ufoController)
    {
        ufoController->update(evt);

        int currentHealth = ufoController->getHealth();
        if (healthLabel && currentHealth != displayedHealth)
        {
            displayedHealth = currentHealth;

            healthLabel->setCaption(
                "UFO HEALTH: " +
                std::to_string(displayedHealth));
        }

        int currentCows = ufoController->getCowsAbducted();
        if (cowsLabel && currentCows != displayedCows)
        {
            displayedCows = currentCows;
            cowsLabel->setCaption(
                "COWS ABDUCTED: " + std::to_string(displayedCows));
        }

        int currentTimerSeconds = static_cast<int>(
            std::ceil(ufoController->getGameTimer()));
        if (timerLabel && currentTimerSeconds != displayedTimerSeconds)
        {
            displayedTimerSeconds = currentTimerSeconds;
            timerLabel->setCaption(
                "TIME LEFT: " + formatTimer(displayedTimerSeconds));
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

bool CoreApp::mouseMoved(const OgreBites::MouseMotionEvent &evt)
{
    return true;
}

bool CoreApp::mouseWheelRolled(const OgreBites::MouseWheelEvent &evt)
{
    return true;
}

bool CoreApp::mousePressed(const OgreBites::MouseButtonEvent &evt)
{
    return true;
}
