
#include "UFOController.h"
#include "SpawnManager.h"


UFOController::UFOController(
    Ogre::SceneManager* scnMgr,
    Ogre::SceneNode* playerRoot,
    Ogre::SceneNode* visualNode,
    Ogre::Camera* camera,
    ParticleManager* particleMgr,
    AudioManager* audioManager
)
    : mScnMgr(scnMgr),
      mPlayerRoot(playerRoot),
      mVisualNode(visualNode),
      mParticleManager(particleMgr),
      mAudioManager(audioManager),
      mSpawnManager(nullptr),
      mCowsAbducted(0),
      mHealth(5),
      mIsDead(false),
      mInvulnerabilityTimer(0.0f),
      mHoverAltitude(30.0f),
      mGoingForward(false),
      mGoingBack(false),
      mGoingLeft(false),
      mGoingRight(false),
      mBaseSpeed(100.0f),
      mCurrentSpeed(100.0f),
      isBoosting(false),
      mSpotlight(nullptr),
      mFillLight(nullptr),
      mSpotlightNode(nullptr),
      mGameTimer(60.0f),
      mTimeIsUp(false),
      mPrinted30Sec(false),
      mPrinted10Sec(false)
      // CHANGE mGameTimer to 180.0f LATER
{
    std::random_device rd;
    mRng = std::mt19937(rd());

    mSpeedDistribution =
        std::uniform_real_distribution<float>(
            20.0f, 350.0f
        );

    // Camera boom
    mCameraBoom =
        mPlayerRoot->createChildSceneNode("CameraBoom");

    mCameraBoom->setPosition(0, 80, 100);
    mCameraBoom->attachObject(camera);

    mCameraBoom->lookAt(
        Ogre::Vector3(0, 0, 0),
        Ogre::Node::TS_PARENT
    );

    // Capture raycast
    mRaySceneQuery =
        mScnMgr->createRayQuery(Ogre::Ray());

    // Create the light object
    mSpotlight = mScnMgr->createLight("UFOSpotlight");
    mSpotlight->setType(Ogre::Light::LT_SPOTLIGHT);
    
    mSpotlight->setDiffuseColour(1.5f, 1.08f, 0.68f);
    mSpotlight->setSpecularColour(1.5f, 1.18f, 0.82f);
    mSpotlight->setCastShadows(true);
    
    // Define the cone of the spotlight
    // Inner angle (fully bright) and Outer angle (fades to black)
    mSpotlight->setSpotlightRange(
        Ogre::Degree(16.0f), Ogre::Degree(50.0f), 0.8f);
    
    // Set the attenuation (range and falloff)
    mSpotlight->setAttenuation(600.0f, 1.0f, 0.0045f, 0.0f);

    // Attach it to a new node on the Player Root
    mSpotlightNode = mPlayerRoot->createChildSceneNode("UFOSpotlightNode");
    mSpotlightNode->attachObject(mSpotlight);

    mFillLight = mScnMgr->createLight("UFODesertFillLight");
    mFillLight->setType(Ogre::Light::LT_POINT);
    mFillLight->setDiffuseColour(0.5f, 0.28f, 0.12f);
    mFillLight->setSpecularColour(0.08f, 0.04f, 0.015f);
    mFillLight->setAttenuation(400.0f, 1.0f, 0.0025f, 0.0f);
    mSpotlightNode->attachObject(mFillLight);
    
    mSpotlightNode->setDirection(Ogre::Vector3(0, -1, 0), Ogre::Node::TS_PARENT);
}

UFOController::~UFOController()
{
    if (mRaySceneQuery)
    {
        mScnMgr->destroyQuery(mRaySceneQuery);
        mRaySceneQuery = nullptr;
    }
}

void UFOController::setSpawnManager(SpawnManager* manager)
{
    mSpawnManager = manager;
}

float UFOController::getTerrainHeight(float x, float z) const
{
    // Preserving existing hover behavior for now
    return 0.0f;
}

void UFOController::takeDamage(int amount)
{
    // Ignore damage if dead, invulnerable, or if the game timer has ended
    if (mIsDead || mTimeIsUp || mInvulnerabilityTimer > 0.0f) return;

    mHealth -= amount;
    mInvulnerabilityTimer = 1.5f;

    if (mHealth <= 0)
    {
        mIsDead = true;
        mVisualNode->setVisible(false);

        // Turn off the spotlight
        mSpotlight->setVisible(false);
        mFillLight->setVisible(false);

        mAudioManager->playSFX("explode.wav");

        if (mParticleManager)
        {
            mParticleManager->spawnExplosion(
                mPlayerRoot->getPosition(),
                "UFO/Explosion",
                1.5f
            );
        }

        Ogre::LogManager::getSingleton().logMessage(
            "GAME OVER: UFO Destroyed!"
        );
    }
    else
    {
        mAudioManager->playSFX("explodemini.wav");

        if (mParticleManager)
        {
            mParticleManager->spawnExplosion(
                mPlayerRoot->getPosition(),
                "UFO/SmallExplosion",
                0.5f
            );
        }

        Ogre::LogManager::getSingleton().logMessage(
            "UFO Hit! HP remaining: " +
            std::to_string(mHealth)
        );
    }
}

void UFOController::update(const Ogre::FrameEvent& evt)
{
    if (!mPlayerRoot || !mVisualNode)
        return;

    // --- Timer Logic ---
    if (!mTimeIsUp && !mIsDead) 
    {
        mGameTimer -= evt.timeSinceLastFrame;

        // Print debug messages at specific thresholds
        if (mGameTimer <= 30.0f && !mPrinted30Sec) {
            Ogre::LogManager::getSingleton().logMessage("DEBUG: 30 seconds left!");
            mPrinted30Sec = true;
        }
        if (mGameTimer <= 10.0f && !mPrinted10Sec) {
            Ogre::LogManager::getSingleton().logMessage("DEBUG: 10 seconds left!");
            mPrinted10Sec = true;
        }

        // Handle Time Over state
        if (mGameTimer <= 0.0f) {
            mGameTimer = 0.0f;
            mTimeIsUp = true;
            
            // Halt any current directional movement
            mGoingForward = mGoingBack = mGoingLeft = mGoingRight = false;
            
            Ogre::LogManager::getSingleton().logMessage("TIME IS UP!");
            Ogre::LogManager::getSingleton().logMessage("Final Score: Cows Abducted = " + std::to_string(mCowsAbducted));
        }
    }

    // If dead, bypass all movement, terrain following, and spinning logic
    if (mIsDead)
        return;

    float dt = evt.timeSinceLastFrame;

    if (mInvulnerabilityTimer > 0.0f)
        mInvulnerabilityTimer -= dt;


    // UFO visual rotation
    mVisualNode->yaw(Ogre::Degree(10.0f * dt));

    // Only allow movement while time remains
    if (!mTimeIsUp)
    {
        Ogre::Vector3 moveDir = Ogre::Vector3::ZERO;

        if (mGoingForward) moveDir.z -= 1.0f;
        if (mGoingBack)    moveDir.z += 1.0f;
        if (mGoingLeft)    moveDir.x -= 1.0f;
        if (mGoingRight)   moveDir.x += 1.0f;

        if (moveDir != Ogre::Vector3::ZERO)
        {
            moveDir.normalise();

            mPlayerRoot->translate(
                moveDir * mCurrentSpeed * dt,
                Ogre::Node::TS_WORLD
            );
        }
    }

    

    // Calculate movement direction
    Ogre::Vector3 moveDir(Ogre::Vector3::ZERO);

    // Standard 3D plane movement (X/Z plane)
    if (mGoingForward)
        moveDir.z -= 1.0f;
    if (mGoingBack)
        moveDir.z += 1.0f;
    if (mGoingLeft)
        moveDir.x -= 1.0f;
    if (mGoingRight)
        moveDir.x += 1.0f;

    // Normalize the vector so diagonal movement isn't faster than straight movement
    moveDir.normalise();

    // Apply continuous translation based on speed and time delta
    if (moveDir != Ogre::Vector3::ZERO)
    {
        mPlayerRoot->translate(moveDir * mCurrentSpeed * evt.timeSinceLastFrame, Ogre::Node::TS_WORLD);
    }

    Ogre::Vector3 currentPos = mPlayerRoot->getPosition();

    // --- Infinite World Wrapping Logic (-500 to 499 range) ---
    float minCoord = -500.0f;
    float maxCoord = 500.0f; // wrap threshold (exclusive)
    float span = 1000.0f;

    if (currentPos.x < minCoord)
    {
        currentPos.x += span;
    }
    else if (currentPos.x >= maxCoord)
    {
        currentPos.x -= span;
    }

    if (currentPos.z < minCoord)
    {
        currentPos.z += span;
    }
    else if (currentPos.z >= maxCoord)
    {
        currentPos.z -= span;
    }
    // ---------------------------------------------------------

    // --- Terrain Following Logic ---
    // Convert world coordinates (-500 to 499) into local heightmap coordinates (0 to 999)
    float localX = currentPos.x + 500.0f;
    float localZ = currentPos.z + 500.0f;

    float surfaceY = getTerrainHeight(localX, localZ);

    // Clamp the UFO's Y position to the terrain surface plus the hover altitude
    currentPos.y = surfaceY + mHoverAltitude;

    mPlayerRoot->setPosition(currentPos);
}

void UFOController::injectKeyDown(
    const OgreBites::KeyboardEvent& evt
)
{
    // Ignore all input (Movement, Hyperspeed, Abduction) if dead or time is up
    if (mIsDead || mTimeIsUp) return;

    OgreBites::Keycode key = evt.keysym.sym;

    if (key == 'w')
        mGoingForward = true;

    else if (key == 's')
        mGoingBack = true;

    else if (key == 'a')
        mGoingLeft = true;

    else if (key == 'd')
        mGoingRight = true;

    else if (key == OgreBites::SDLK_LSHIFT && !isBoosting)
    {
        mCurrentSpeed = mSpeedDistribution(mRng);
        mAudioManager->playSFX("ufo_boost.flac");
        isBoosting = true;
    }

    else if (key == OgreBites::SDLK_SPACE)
    {

        mSpotlight->setDiffuseColour(0.3f, 0.9f, 3.0f);
        mSpotlight->setSpecularColour(0.3f, 0.9f, 3.0f);

        if (!mSpawnManager)
            return;

        Ogre::Vector3 ufoPos =
            mPlayerRoot->getPosition();

        Ogre::Ray downwardRay(
            ufoPos,
            Ogre::Vector3::NEGATIVE_UNIT_Y
        );

        mRaySceneQuery->setRay(downwardRay);
        mRaySceneQuery->setSortByDistance(true);
        mRaySceneQuery->setQueryMask(COW_MASK);

        Ogre::RaySceneQueryResult& result =
            mRaySceneQuery->execute();

        for (auto itr = result.begin();
             itr != result.end(); ++itr)
        {
            if (!itr->movable)
                continue;

            if (itr->distance >= 150.0f)
                continue;

            // Extra cow flag verification
            if ((itr->movable->getQueryFlags() & COW_MASK) == 0)
                continue;

            Ogre::SceneNode* cowNode =
                itr->movable->getParentSceneNode();

            if (!cowNode)
                continue;

            // Begin capture animation
            mSpawnManager->captureCow(cowNode);

            mCowsAbducted++;

            Ogre::LogManager::getSingleton().logMessage(
                "Cow abducted! Total: " +
                std::to_string(mCowsAbducted)
            );

            mAudioManager->playSFX("ufo_abduct.flac");

            break;
        }
    }


    else if (key == 'e')
    {
        takeDamage(1);
    }

    
}

void UFOController::injectKeyUp(
    const OgreBites::KeyboardEvent& evt
)
{
    // Ignore key releases to prevent state bugs if time runs out while a key is held
    if (mIsDead || mTimeIsUp) return;
    
    OgreBites::Keycode key = evt.keysym.sym;

    if (key == 'w')
        mGoingForward = false;

    else if (key == 's')
        mGoingBack = false;

    else if (key == 'a')
        mGoingLeft = false;

    else if (key == 'd')
        mGoingRight = false;

    else if (key == OgreBites::SDLK_LSHIFT){
        mCurrentSpeed = mBaseSpeed;
        isBoosting = false;
    }
    else if (key == OgreBites::SDLK_SPACE){
        mSpotlight->setDiffuseColour(1.5f, 1.08f, 0.68f);
        mSpotlight->setSpecularColour(1.5f, 1.18f, 0.82f);
    }
}
