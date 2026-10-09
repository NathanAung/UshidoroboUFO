#ifndef __UFO_CONTROLLER_H__
#define __UFO_CONTROLLER_H__

#include <Ogre.h>
#include <OgreInput.h>
#include <random>

class UFOController {
public:
    UFOController(Ogre::SceneManager* scnMgr, Ogre::SceneNode* playerRoot, Ogre::SceneNode* visualNode, Ogre::Camera* camera);
    ~UFOController();

    // Core update loop for continuous actions
    void update(const Ogre::FrameEvent& evt);

    // Input hooks delegated from CoreApp
    void injectKeyDown(const OgreBites::KeyboardEvent& evt);
    void injectKeyUp(const OgreBites::KeyboardEvent& evt);

private:
    Ogre::SceneManager* mScnMgr;
    
    // Split node architecture
    Ogre::SceneNode* mPlayerRoot; // Translates the player and camera
    Ogre::SceneNode* mVisualNode; // Spins the 3D model
    Ogre::SceneNode* mCameraBoom; // Holds the camera offset

    // Movement state flags
    bool mGoingForward;
    bool mGoingBack;
    bool mGoingLeft;
    bool mGoingRight;

    // Base movement speed (units per second)
    float mBaseSpeed;
    float mCurrentSpeed; // Tracks the active speed multiplier

    // C++11 Random Number Generation
    std::mt19937 mRng;
    std::uniform_real_distribution<float> mSpeedDistribution;
};

#endif