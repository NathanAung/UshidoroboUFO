
#include "CoreApp.h"
#include <cmath>

CoreApp::CoreApp()
    : OgreBites::ApplicationContext("OgreTestApp"),
      ufoController(nullptr),
      spawnManager(nullptr),
      trayManager(nullptr),
      mParticleManager(nullptr),
      healthLabel(nullptr),
    timerLabel(nullptr),
    cowCountLabel(nullptr),
    displayedHealth(-1),
    displayedTimeSeconds(-1),
    displayedCowCount(-1)
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

    
    // Initialize ParticleManager
    mParticleManager = new ParticleManager(scnMgr);

    // Initialize SpawnManager
    spawnManager = new SpawnManager(scnMgr);

    // Tell SpawnManager which node is the UFO
    spawnManager->setUFONode(playerRoot);

    // Create UFOController ONCE
    ufoController = new UFOController(
        scnMgr,
        playerRoot,
        visualNode,
        cam,
        mParticleManager
    );

    // Connect SpawnManager to UFOController
    ufoController->setSpawnManager(spawnManager);


    // Spawn test cows
    spawnManager->spawnRandom(SpawnType::Cow, 50);

    trayManager = new OgreBites::TrayManager("GameUI", getRenderWindow(), this);
    trayManager -> hideCursor();
    addInputListener(trayManager);
    displayedHealth = ufoController->getHealth();
    healthLabel = trayManager->createLabel(
        OgreBites::TL_TOPLEFT, "HealthLabel",
        "UFO HEALTH: " + std::to_string(displayedHealth), 220);
    displayedTimeSeconds = static_cast<int>(std::ceil(ufoController->getRemainingTime()));
    timerLabel = trayManager->createLabel(
        OgreBites::TL_TOPRIGHT, "TimerLabel", "TIME: 01:00", 220);
    displayedCowCount = ufoController->getCowsAbducted();
    cowCountLabel = trayManager->createLabel(
        OgreBites::TL_TOPRIGHT, "CowCountLabel",
        "COWS: " + std::to_string(displayedCowCount), 220);
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

        int remainingSeconds = static_cast<int>(
            std::ceil(ufoController->getRemainingTime())
        );
        if (timerLabel && remainingSeconds != displayedTimeSeconds)
        {
            displayedTimeSeconds = remainingSeconds;
            int minutes = remainingSeconds / 60;
            int seconds = remainingSeconds % 60;
            timerLabel->setCaption(
                "TIME: " + std::to_string(minutes) + ":" +
                (seconds < 10 ? "0" : "") + std::to_string(seconds)
            );
        }

        int currentCowCount = ufoController->getCowsAbducted();
        if (cowCountLabel && currentCowCount != displayedCowCount)
        {
            displayedCowCount = currentCowCount;
            cowCountLabel->setCaption(
                "COWS: " + std::to_string(displayedCowCount)
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
