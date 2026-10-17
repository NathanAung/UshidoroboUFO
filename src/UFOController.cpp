
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
      mCurrentSpeed(100.0f)
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
    if (mIsDead || mInvulnerabilityTimer > 0.0f)
        return;

    mHealth -= amount;
    mInvulnerabilityTimer = 1.5f;

    if (mHealth <= 0)
    {
        mIsDead = true;
        mVisualNode->setVisible(false);

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

    if (mIsDead)
        return;

    float dt = evt.timeSinceLastFrame;

    if (mInvulnerabilityTimer > 0.0f)
        mInvulnerabilityTimer -= dt;

    // UFO visual rotation
    mVisualNode->yaw(Ogre::Degree(10.0f * dt));

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
    if (mIsDead)
        return;

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
    if (mIsDead)
        return;

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
