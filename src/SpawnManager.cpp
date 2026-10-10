#include "SpawnManager.h"
#include "UFOController.h" // For QueryFlags

#include <string>

SpawnManager::SpawnManager(Ogre::SceneManager* scnMgr)
    : mScnMgr(scnMgr),
      mRng(std::random_device{}()),
      mPositionDistribution(-500.0f, 500.0f)
{
}

SpawnManager::~SpawnManager()
{
    for (Ogre::SceneNode* node : mSpawnedNodes)
    {
        node->detachAllObjects();
        mScnMgr->destroySceneNode(node);
    }

    for (Ogre::Entity* entity : mSpawnedEntities)
    {
        mScnMgr->destroyEntity(entity);
    }
}

float SpawnManager::getTerrainHeight(float x, float z) const
{
    // Temporary flat terrain
    return 0.0f;
}

Ogre::SceneNode* SpawnManager::spawnObject(
    SpawnType type,
    const Ogre::Vector3& position)
{
    std::string name = "Spawn_" + std::to_string(mSpawnCounter++);

    // Placeholder mesh for all objects
    Ogre::Entity* entity = mScnMgr->createEntity(
        name,
        "ogrehead.mesh"
    );

    switch (type)
    {
    case SpawnType::Cow:
        entity->setQueryFlags(COW_MASK);
        break;

    case SpawnType::Tank:
    case SpawnType::Helicopter:
        entity->setQueryFlags(ENEMY_MASK);
        break;

    case SpawnType::MilitaryBase:
        entity->setQueryFlags(ENVIRONMENT_MASK);
        break;
    }

    Ogre::SceneNode* node =
        mScnMgr->getRootSceneNode()->createChildSceneNode();

    node->attachObject(entity);
    node->setPosition(position);

    // Temporary scale
    node->setScale(0.5f, 0.5f, 0.5f);

    mSpawnedNodes.push_back(node);
    mSpawnedEntities.push_back(entity);

    return node;
}

void SpawnManager::spawnRandom(SpawnType type, int count)
{
    for (int i = 0; i < count; i++)
    {
        float x = mPositionDistribution(mRng);
        float z = mPositionDistribution(mRng);

        float y = getTerrainHeight(x, z);

        // Helicopters fly above terrain
        if (type == SpawnType::Helicopter)
        {
            y += 80.0f;
        }

        spawnObject(type, Ogre::Vector3(x, y, z));
    }
}