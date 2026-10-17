#ifndef __UFO_CONTROLLER_H__
#define __UFO_CONTROLLER_H__

#include <Ogre.h>
#include <OgreInput.h>
#include <random>
#include "ParticleManager.h"
#include "SpawnManager.h"

// Define query masks to differentiate entities in the game world
enum QueryFlags {
    COW_MASK = 1 << 0,
    ENEMY_MASK = 1 << 1,
    ENVIRONMENT_MASK = 1 << 2,
    PLAYER_MASK = 1 << 3
};

class SpawnManager;

class UFOController {
public:
    UFOController(Ogre::SceneManager* scnMgr, Ogre::SceneNode* playerRoot, Ogre::SceneNode* visualNode, Ogre::Camera* camera, ParticleManager* particleMgr);
    ~UFOController();

    // Core update loop for continuous actions
    void update(const Ogre::FrameEvent& evt);

    // Input hooks delegated from CoreApp
    void injectKeyDown(const OgreBites::KeyboardEvent& evt);
    void injectKeyUp(const OgreBites::KeyboardEvent& evt);

    int getCowsAbducted() const { return mCowsAbducted; }
    int getHealth() const { return mHealth; }
    float getRemainingTime() const { return mGameTimer; }

    // Allows enemy AI to deal damage to the UFO
    void takeDamage(int amount);

    void setSpawnManager(SpawnManager* manager);

private:
    Ogre::SceneManager* mScnMgr;
    ParticleManager* mParticleManager;
    
    // Split node architecture
    Ogre::SceneNode* mPlayerRoot; // Translates the player and camera
    Ogre::SceneNode* mVisualNode; // Spins the 3D model
    Ogre::SceneNode* mCameraBoom; // Holds the camera offset

    // Raycast system for the abduction beam
    Ogre::RaySceneQuery* mRaySceneQuery;
    
    int mCowsAbducted;
    int mHealth;
    float mInvulnerabilityTimer;
    float mHoverAltitude; // The distance the UFO hovers above the ground

    bool mIsDead;

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

    // timer
    float mGameTimer;
    bool mTimeIsUp;
    bool mPrinted30Sec;
    bool mPrinted10Sec;

    // Mock terrain function to be replaced by terrain generation
    float getTerrainHeight(float x, float z) const;

    SpawnManager* mSpawnManager = nullptr;
};

#endif