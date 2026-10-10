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
};

#endif