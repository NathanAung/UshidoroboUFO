
#include "SpawnManager.h"
#include "UFOController.h"
#include "TerrainController.h"

#include <string>
#include <algorithm>
#include <cmath>

SpawnManager::SpawnManager(Ogre::SceneManager* scnMgr)
    : mScnMgr(scnMgr),
      mRng(std::random_device{}()),
      mPositionDistribution(20.0f, 979.0f)
{
}

SpawnManager::~SpawnManager()
{
    for (Ogre::SceneNode* node : mSpawnedNodes)
    {
        if (node)
        {
            node->detachAllObjects();
            mScnMgr->destroySceneNode(node);
        }
    }

    for (Ogre::Entity* entity : mSpawnedEntities)
    {
        if (entity)
            mScnMgr->destroyEntity(entity);
    }
}

float SpawnManager::getTerrainHeight(float x, float z) const
{
    return TerrainController::getHeightAt(x, z);
}

SpawnManager::SpawnConfig SpawnManager::getSpawnConfig(
    SpawnType type
) const
{
    switch (type)
    {
    case SpawnType::Cow:
        return {
            "ogrehead.mesh",
            Ogre::Vector3(0.25f, 0.25f, 0.25f)
        };

    case SpawnType::Tank:
        return {
            "ogrehead.mesh",
            Ogre::Vector3(1.0f, 1.0f, 1.0f)
        };

    case SpawnType::Helicopter:
        return {
            "ogrehead.mesh",
            Ogre::Vector3(2.0f, 2.0f, 2.0f)
        };

    case SpawnType::MilitaryBase:
        return {
            "ogrehead.mesh",
            Ogre::Vector3(5.0f, 5.0f, 5.0f)
        };

    default:
        return {
            "ogrehead.mesh",
            Ogre::Vector3::UNIT_SCALE
        };
    }
}

Ogre::SceneNode* SpawnManager::spawnObject(
    SpawnType type,
    const Ogre::Vector3& position
)
{
    std::string name =
        "Spawn_" + std::to_string(mSpawnCounter++);

    SpawnConfig config = getSpawnConfig(type);

    Ogre::Entity* entity =
        mScnMgr->createEntity(name, config.meshName);

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
    node->setScale(config.scale);

    mSpawnedNodes.push_back(node);
    mSpawnedEntities.push_back(entity);

    return node;
}

void SpawnManager::spawnRandom(SpawnType type, int count)
{
    for (int i = 0; i < count; ++i)
    {
        float x = mPositionDistribution(mRng);
        float z = mPositionDistribution(mRng);

        float y = getTerrainHeight(x, z);

        if (type == SpawnType::Helicopter)
            y += 80.0f;

        spawnObject(
            type,
            Ogre::Vector3(x, y, z)
        );
    }
}

void SpawnManager::captureCow(Ogre::SceneNode* cowNode)
{
    if (!cowNode)
        return;

    // Only accept nodes owned by SpawnManager
    auto ownedNode = std::find(
        mSpawnedNodes.begin(),
        mSpawnedNodes.end(),
        cowNode
    );

    if (ownedNode == mSpawnedNodes.end())
        return;

    // Prevent duplicate captures
    for (const auto& cow : mCapturingCows)
    {
        if (cow.node == cowNode)
            return;
    }

    // Ensure this node actually contains a cow
    bool isCow = false;

    for (unsigned int i = 0;
         i < cowNode->numAttachedObjects(); ++i)
    {
        Ogre::MovableObject* object =
            cowNode->getAttachedObject(i);

        if ((object->getQueryFlags() & COW_MASK) != 0)
            isCow = true;
    }

    if (!isCow)
        return;

    CapturedCow cow;
    cow.node = cowNode;
    cow.startScale = cowNode->getScale();
    cow.startPosition = cowNode->getPosition();

    mCapturingCows.push_back(cow);

    // Prevent additional raycast hits
    for (unsigned int i = 0;
         i < cowNode->numAttachedObjects(); ++i)
    {
        cowNode->getAttachedObject(i)->setQueryFlags(0);
    }
}

void SpawnManager::update(float deltaTime)
{
    for (auto it = mCapturingCows.begin();
         it != mCapturingCows.end();)
    {
        CapturedCow& cow = *it;

        cow.elapsed += deltaTime;

        float t = std::min(
            cow.elapsed / mCaptureDuration,
            1.0f
        );

        // Spin
        cow.node->yaw(
            Ogre::Degree(mCaptureSpinSpeed * deltaTime)
        );

        // Shrink
        float shrink = 1.0f - t;
        cow.node->setScale(cow.startScale * shrink);

        // Move toward UFO
        if (mUFONode)
        {
            Ogre::Vector3 targetPosition =
                mUFONode->_getDerivedPosition();

            // Move toward the UFO's underside
            targetPosition.y -= 5.0f;

            // Smoothly interpolate from cow to UFO
            float smoothT = t * t * (3.0f - 2.0f * t);

            Ogre::Vector3 newPosition =
                cow.startPosition +
                (targetPosition - cow.startPosition) * smoothT;

            cow.node->setPosition(newPosition);
        }

        if (t >= 1.0f)
        {
            Ogre::SceneNode* node = cow.node;

            // Detach and destroy entities
            while (node->numAttachedObjects() > 0)
            {
                Ogre::MovableObject* object =
                    node->getAttachedObject(0);

                node->detachObject(object);

                Ogre::Entity* entity =
                    static_cast<Ogre::Entity*>(object);

                auto entityIt = std::find(
                    mSpawnedEntities.begin(),
                    mSpawnedEntities.end(),
                    entity
                );

                if (entityIt != mSpawnedEntities.end())
                    mSpawnedEntities.erase(entityIt);

                mScnMgr->destroyEntity(entity);
            }

            // Remove node from ownership tracking
            auto nodeIt = std::find(
                mSpawnedNodes.begin(),
                mSpawnedNodes.end(),
                node
            );

            if (nodeIt != mSpawnedNodes.end())
                mSpawnedNodes.erase(nodeIt);

            mScnMgr->destroySceneNode(node);

            it = mCapturingCows.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

void SpawnManager::setUFONode(Ogre::SceneNode* ufoNode)
{
    mUFONode = ufoNode;
}