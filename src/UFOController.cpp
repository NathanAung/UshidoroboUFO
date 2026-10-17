
#include "UFOController.h"
#include "SpawnManager.h"


UFOController::UFOController(
    Ogre::SceneManager* scnMgr,
    Ogre::SceneNode* playerRoot,
    Ogre::SceneNode* visualNode,
    Ogre::Camera* camera,
    ParticleManager* particleMgr
)
    : mScnMgr(scnMgr),
      mPlayerRoot(playerRoot),
      mVisualNode(visualNode),
      mParticleManager(particleMgr),
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
    
    // Set the color (e.g., a slightly eerie alien green/cyan). 
    // Format is (Red, Green, Blue) from 0.0 to 1.0.
    mSpotlight->setDiffuseColour(0.4f, 1.0f, 0.8f); 
    mSpotlight->setSpecularColour(1.0f, 1.0f, 1.0f);
    
    // Define the cone of the spotlight
    // Inner angle (fully bright) and Outer angle (fades to black)
    mSpotlight->setSpotlightRange(Ogre::Degree(15.0f), Ogre::Degree(35.0f));
    
    // Set the attenuation (range and falloff)
    // Range: 500 units, Constant: 1.0, Linear: 0.005, Quadratic: 0.0
    mSpotlight->setAttenuation(500.0f, 1.0f, 0.005f, 0.0f);

    // Attach it to a new node on the Player Root
    mSpotlightNode = mPlayerRoot->createChildSceneNode("UFOSpotlightNode");
    mSpotlightNode->attachObject(mSpotlight);
    
    // Point the node directly downwards
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
    if (mIsDead) return; 

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

    

    // Terrain following
    Ogre::Vector3 currentPos = mPlayerRoot->getPosition();

    float surfaceY =
        getTerrainHeight(currentPos.x, currentPos.z);

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

    else if (key == OgreBites::SDLK_LSHIFT)
    {
        mCurrentSpeed = mSpeedDistribution(mRng);
    }

    else if (key == OgreBites::SDLK_SPACE)
    {
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

    else if (key == OgreBites::SDLK_LSHIFT)
        mCurrentSpeed = mBaseSpeed;
}
