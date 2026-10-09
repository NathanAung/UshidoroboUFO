#include "UFOController.h"


UFOController::UFOController(Ogre::SceneManager* scnMgr, Ogre::SceneNode* playerRoot, Ogre::SceneNode* visualNode, Ogre::Camera* camera)
    : mScnMgr(scnMgr), mPlayerRoot(playerRoot), mVisualNode(visualNode),
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
}


UFOController::~UFOController()
{
}


void UFOController::update(const Ogre::FrameEvent& evt)
{
    if (!mPlayerRoot || !mVisualNode) return;

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
}


void UFOController::injectKeyDown(const OgreBites::KeyboardEvent& evt)
{
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
        // Abduction logic goes here
    }
}


void UFOController::injectKeyUp(const OgreBites::KeyboardEvent& evt)
{
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