#include "UFOController.h"


UFOController::UFOController(Ogre::SceneManager* scnMgr, Ogre::SceneNode* playerRoot, Ogre::SceneNode* visualNode, Ogre::Camera* camera, ParticleManager* particleMgr)
    : mScnMgr(scnMgr), mPlayerRoot(playerRoot), mVisualNode(visualNode), mParticleManager(particleMgr),
      mCowsAbducted(0), mHealth(5), mIsDead(false), mInvulnerabilityTimer(0.0f), mHoverAltitude(30.0f),
      mGoingForward(false), mGoingBack(false), mGoingLeft(false), mGoingRight(false),
      mBaseSpeed(100.0f), mCurrentSpeed(100.0f) // Adjust this value to match the game's scale
{
    // Seed the random number generator
    std::random_device rd;
    mRng = std::mt19937(rd());
    
    // Define the hyperspeed range. 
    // The design specifies it can be both fast and slow, so the range spans below and above base speed.
    mSpeedDistribution = std::uniform_real_distribution<float>(20.0f, 350.0f);

    // Create the camera boom attached to the NON-spinning player root
    mCameraBoom = mPlayerRoot->createChildSceneNode("CameraBoom");
    
    // Position the camera offset: 80 units up (Y), 100 units behind (Z)
    mCameraBoom->setPosition(0, 80, 100); 
    
    // Attach the camera to this boom
    mCameraBoom->attachObject(camera);
    
    // Point the camera to look slightly downward at the UFO's local origin
    mCameraBoom->lookAt(Ogre::Vector3(0, 0, 0), Ogre::Node::TS_PARENT);

    // Initialize the RaySceneQuery
    mRaySceneQuery = mScnMgr->createRayQuery(Ogre::Ray());
}


UFOController::~UFOController()
{
    // Clean up the query memory
    if (mRaySceneQuery) {
        mScnMgr->destroyQuery(mRaySceneQuery);
    }
}


// Mock terrain implementation
float UFOController::getTerrainHeight(float x, float z) const
{
    // Return 0 for now. Once the procedural map is finished, 
    // call terrain manager API here (e.g., return TerrainManager::getHeight(x, z);)
    return 0.0f; 
}


void UFOController::takeDamage(int amount)
{
    // Ignore damage if already dead or invulnerable
    if (mIsDead || mInvulnerabilityTimer > 0.0f) return;

    mHealth -= amount;
    mInvulnerabilityTimer = 1.5f; 

    if (mHealth <= 0)
    {
        mIsDead = true;
        
        // Hide the UFO 3D model
        mVisualNode->setVisible(false);
        
        // Trigger the large death explosion (lasts 1.5 seconds)
        if (mParticleManager) {
            mParticleManager->spawnExplosion(mPlayerRoot->getPosition(), "UFO/Explosion", 1.5f);
        }

        Ogre::LogManager::getSingleton().logMessage("GAME OVER: UFO Destroyed!");
    }
    else
    {
        // Trigger the small damage explosion (lasts 0.5 seconds)
        if (mParticleManager) {
            mParticleManager->spawnExplosion(mPlayerRoot->getPosition(), "UFO/SmallExplosion", 0.5f);
        }

        Ogre::LogManager::getSingleton().logMessage("UFO Hit! HP remaining: " + std::to_string(mHealth));
    }
}


void UFOController::update(const Ogre::FrameEvent& evt)
{
    if (!mPlayerRoot || !mVisualNode) return;

    // If dead, bypass all movement, terrain following, and spinning logic
    if (mIsDead) return; 

    // Decrease the invulnerability timer
    if (mInvulnerabilityTimer > 0.0f) {
        mInvulnerabilityTimer -= evt.timeSinceLastFrame;
    }

    // Spin the visual mesh independently
    mVisualNode->yaw(Ogre::Degree(10.0f * evt.timeSinceLastFrame));

    // Calculate movement direction
    Ogre::Vector3 moveDir(Ogre::Vector3::ZERO);

    // Standard 3D plane movement (X/Z plane)
    if (mGoingForward) moveDir.z -= 1.0f;
    if (mGoingBack)    moveDir.z += 1.0f;
    if (mGoingLeft)    moveDir.x -= 1.0f;
    if (mGoingRight)   moveDir.x += 1.0f;

    // Normalize the vector so diagonal movement isn't faster than straight movement
    moveDir.normalise();

    // Apply continuous translation based on speed and time delta
    if (moveDir != Ogre::Vector3::ZERO) {
        mPlayerRoot->translate(moveDir * mCurrentSpeed * evt.timeSinceLastFrame, Ogre::Node::TS_WORLD);
    }

    // --- Terrain Following Logic ---
    Ogre::Vector3 currentPos = mPlayerRoot->getPosition();
    
    // Sample the terrain height directly below the UFO
    float surfaceY = getTerrainHeight(currentPos.x, currentPos.z);
    
    // Clamp the UFO's Y position to the terrain surface plus the hover altitude
    currentPos.y = surfaceY + mHoverAltitude;
    mPlayerRoot->setPosition(currentPos);
}


void UFOController::injectKeyDown(const OgreBites::KeyboardEvent& evt)
{
    if (mIsDead) return;

    OgreBites::Keycode key = evt.keysym.sym;

    if (key == 'w') mGoingForward = true;
    else if (key == 's') mGoingBack = true;
    else if (key == 'a') mGoingLeft = true;
    else if (key == 'd') mGoingRight = true;
    // Trigger Hyperspeed
    else if (key == OgreBites::SDLK_LSHIFT)
    {
        // Sample a new random speed from our distribution
        mCurrentSpeed = mSpeedDistribution(mRng);
    }
    else if (key == OgreBites::SDLK_SPACE)
    {
        // Get current position and cast a ray directly downwards
        Ogre::Vector3 ufoPos = mPlayerRoot->getPosition();
        Ogre::Ray downwardRay(ufoPos, Ogre::Vector3::NEGATIVE_UNIT_Y);
        
        mRaySceneQuery->setRay(downwardRay);
        mRaySceneQuery->setSortByDistance(true);
        
        // Only check collisions against entities flagged as cows
        mRaySceneQuery->setQueryMask(COW_MASK);

        // Execute the raycast
        Ogre::RaySceneQueryResult& result = mRaySceneQuery->execute();
        
        for (auto itr = result.begin(); itr != result.end(); ++itr)
        {
            if (itr->movable)
            {
                // Ensure the cow is within a reasonable capture distance (e.g., 150 units below)
                if (itr->distance < 150.0f) 
                {
                    // Capture the cow
                    Ogre::MovableObject* capturedCow = itr->movable;
                    Ogre::SceneNode* cowNode = capturedCow->getParentSceneNode();
                    
                    // Detach and destroy to remove it from the game world
                    cowNode->detachAllObjects();
                    mScnMgr->destroyEntity(static_cast<Ogre::Entity*>(capturedCow));
                    mScnMgr->destroySceneNode(cowNode);
                    
                    mCowsAbducted++;
                    Ogre::LogManager::getSingleton().logMessage("Cow abducted! Total: " + std::to_string(mCowsAbducted));
                    
                    // Break after capturing one cow so a single beam doesn't capture stacked cows simultaneously
                    break; 
                }
            }
        }
    }
    // DAMAGE TEST
    else if (key == 'e'){
        takeDamage(1);
    }
}


void UFOController::injectKeyUp(const OgreBites::KeyboardEvent& evt)
{
    if (mIsDead) return;
    
    OgreBites::Keycode key = evt.keysym.sym;

    if (key == 'w') mGoingForward = false;
    else if (key == 's') mGoingBack = false;
    else if (key == 'a') mGoingLeft = false;
    else if (key == 'd') mGoingRight = false;
    // Revert Hyperspeed
    else if (key == OgreBites::SDLK_LSHIFT)
    {
        // Return to normal movement speed on key release
        mCurrentSpeed = mBaseSpeed;
    }
}