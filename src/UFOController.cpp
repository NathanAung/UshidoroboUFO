#include "UFOController.h"


UFOController::UFOController(Ogre::SceneManager* scnMgr, Ogre::SceneNode* ufoNode)
    : mScnMgr(scnMgr), mUFONode(ufoNode),
      mGoingForward(false), mGoingBack(false), mGoingLeft(false), mGoingRight(false),
      mBaseSpeed(100.0f) // Adjust this value to match your game's scale
{
}


UFOController::~UFOController()
{
}


void UFOController::update(const Ogre::FrameEvent& evt)
{
    if (!mUFONode) return;

    // Continue the spinning animation
    mUFONode->yaw(Ogre::Degree(10.0f * evt.timeSinceLastFrame));

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
        mUFONode->translate(moveDir * mBaseSpeed * evt.timeSinceLastFrame, Ogre::Node::TS_WORLD);
    }
}


void UFOController::injectKeyDown(const OgreBites::KeyboardEvent& evt)
{
    OgreBites::Keycode key = evt.keysym.sym;

    if (key == 'w') mGoingForward = true;
    else if (key == 's') mGoingBack = true;
    else if (key == 'a') mGoingLeft = true;
    else if (key == 'd') mGoingRight = true;
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
}