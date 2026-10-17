#ifndef __SPAWN_MANAGER_H__
#define __SPAWN_MANAGER_H__

#include <Ogre.h>
#include <random>
#include <vector>

enum class SpawnType
{
    Cow,
    MilitaryBase,
    Tank,
    Helicopter
};

class SpawnManager
{
public:
    SpawnManager(Ogre::SceneManager* scnMgr);
    ~SpawnManager();

    void spawnRandom(SpawnType type, int count);

    void captureCow(Ogre::SceneNode* cowNode);
    
    void update(float deltaTime);

    void setUFONode(Ogre::SceneNode* ufoNode);

private:
    Ogre::SceneManager* mScnMgr;

    std::mt19937 mRng;
    std::uniform_real_distribution<float> mPositionDistribution;

    int mSpawnCounter = 0;

    std::vector<Ogre::SceneNode*> mSpawnedNodes;
    std::vector<Ogre::Entity*> mSpawnedEntities;

    Ogre::SceneNode* spawnObject(
        SpawnType type,
        const Ogre::Vector3& position
    );

    float getTerrainHeight(float x, float z) const;

    struct SpawnConfig
    {
        std::string meshName;
        Ogre::Vector3 scale;
    };

    SpawnConfig getSpawnConfig(SpawnType type) const;

    struct CapturedCow
    {
        Ogre::SceneNode* node;
        Ogre::Vector3 startScale;
        Ogre::Vector3 startPosition;
        float elapsed = 0.0f;
    };

    std::vector<CapturedCow> mCapturingCows;

    float mCaptureDuration = 1.5f;
    float mCaptureSpinSpeed = 720.0f; // Degrees per second
    float mCaptureRiseHeight = 20.0f;

    Ogre::SceneNode* mUFONode = nullptr;

    
    // Movement and behavior data for every cow
    struct Cow
    {
        Ogre::SceneNode* node = nullptr;

        Ogre::Vector3 direction = Ogre::Vector3::UNIT_Z;

        float moveSpeed = 5.0f;
        float stateTimer = 0.0f;

        bool isMoving = true;
    };

    std::vector<Cow> mCows;

    void updateCows(float deltaTime);

    
};

#endif