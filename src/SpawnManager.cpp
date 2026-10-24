
#include "SpawnManager.h"
#include "UFOController.h"
#include "TerrainController.h"
#include "ParticleManager.h"

#include <string>
#include <algorithm>
#include <cmath>

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
    SpawnType type) const
{
    switch (type)
    {
    case SpawnType::Cow:
        return {
            "cow.mesh",
            Ogre::Vector3(6.f, 6.f, 6.f)};

    case SpawnType::Tank:
        return {
            "cartoon_tank.mesh",
            Ogre::Vector3(6.0f, 6.0f, 6.0f)};

    case SpawnType::Helicopter:
        return {
            "ogrehead.mesh",
            Ogre::Vector3(2.0f, 2.0f, 2.0f)};

    case SpawnType::MilitaryBase:
        return {
            "ogrehead.mesh",
            Ogre::Vector3(5.0f, 5.0f, 5.0f)};

    default:
        return {
            "ogrehead.mesh",
            Ogre::Vector3::UNIT_SCALE};
    }
}

Ogre::SceneNode *SpawnManager::spawnObject(
    SpawnType type,
    const Ogre::Vector3 &position)
{
    std::string name =
        "Spawn_" + std::to_string(mSpawnCounter++);

    SpawnConfig config = getSpawnConfig(type);

    Ogre::Entity *entity =
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

    Ogre::SceneNode *node =
        mScnMgr->getRootSceneNode()->createChildSceneNode();

    node->attachObject(entity);
    node->setPosition(position);
    node->setScale(config.scale);

    mSpawnedNodes.push_back(node);
    mSpawnedEntities.push_back(entity);

    // Register every spawned cow for automatic movement
    if (type == SpawnType::Cow)
    {
        Cow cow;
        cow.node = node;

        std::uniform_real_distribution<float> angleDist(
            0.0f, Ogre::Math::TWO_PI);

        std::uniform_real_distribution<float> speedDist(
            3.0f, 7.0f);

        std::uniform_real_distribution<float> timeDist(
            2.0f, 5.0f);

        float angle = angleDist(mRng);

        cow.direction = Ogre::Vector3(
            std::sin(angle),
            0.0f,
            std::cos(angle));

        cow.moveSpeed = speedDist(mRng);
        cow.stateTimer = timeDist(mRng);

        mCows.push_back(cow);
    }

    if (type == SpawnType::Tank)
    {
        Tank tank;
        tank.node = node;

        mTanks.push_back(tank);
    }
    return node;
}

void SpawnManager::spawnRandom(SpawnType type, int count)
{
    for (int i = 0; i < count; ++i)
    {
        float x = mPositionDistribution(mRng);
        float z = mPositionDistribution(mRng);

        float y = getTerrainHeight(x, z);

        if (type == SpawnType::Tank)
            y += 5.0f; // Adjust based on tank mesh size

        if (type == SpawnType::Helicopter)
            y += 80.0f;

        spawnObject(
            type,
            Ogre::Vector3(x, y, z));
    }
}

void SpawnManager::captureCow(Ogre::SceneNode *cowNode)
{
    if (!cowNode)
        return;

    // Only accept nodes owned by SpawnManager
    auto ownedNode = std::find(
        mSpawnedNodes.begin(),
        mSpawnedNodes.end(),
        cowNode);

    if (ownedNode == mSpawnedNodes.end())
        return;

    // Prevent duplicate captures
    for (const auto &cow : mCapturingCows)
    {
        if (cow.node == cowNode)
            return;
    }

    // Ensure this node actually contains a cow
    bool isCow = false;

    for (unsigned int i = 0;
         i < cowNode->numAttachedObjects(); ++i)
    {
        Ogre::MovableObject *object =
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

    // Remove captured cow from normal movement
    mCows.erase(
        std::remove_if(
            mCows.begin(),
            mCows.end(),
            [cowNode](const Cow &cow)
            {
                return cow.node == cowNode;
            }),
        mCows.end());

    // Prevent additional raycast hits
    for (unsigned int i = 0;
         i < cowNode->numAttachedObjects(); ++i)
    {
        cowNode->getAttachedObject(i)->setQueryFlags(0);
    }
}

void SpawnManager::update(float deltaTime)
{
    // Update all cows before processing capture animations
    updateCows(deltaTime);
    // Update all tanks
    updateTanks(deltaTime);
    for (auto it = mCapturingCows.begin();
         it != mCapturingCows.end();)
    {
        CapturedCow &cow = *it;

        cow.elapsed += deltaTime;

        float t = std::min(
            cow.elapsed / mCaptureDuration,
            1.0f);

        // Spin
        cow.node->yaw(
            Ogre::Degree(mCaptureSpinSpeed * deltaTime));

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
            Ogre::SceneNode *node = cow.node;

            // Detach and destroy entities
            while (node->numAttachedObjects() > 0)
            {
                Ogre::MovableObject *object =
                    node->getAttachedObject(0);

                node->detachObject(object);

                Ogre::Entity *entity =
                    static_cast<Ogre::Entity *>(object);

                auto entityIt = std::find(
                    mSpawnedEntities.begin(),
                    mSpawnedEntities.end(),
                    entity);

                if (entityIt != mSpawnedEntities.end())
                    mSpawnedEntities.erase(entityIt);

                mScnMgr->destroyEntity(entity);
            }

            // Remove node from ownership tracking
            auto nodeIt = std::find(
                mSpawnedNodes.begin(),
                mSpawnedNodes.end(),
                node);

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

void SpawnManager::updateCows(float deltaTime)
{
    std::uniform_real_distribution<float> walkTime(2.0f, 5.0f);
    std::uniform_real_distribution<float> restTime(1.0f, 3.0f);
    std::uniform_real_distribution<float> angleDist(
        -Ogre::Math::PI,
        Ogre::Math::PI);

    for (Cow &cow : mCows)
    {
        if (!cow.node)
            continue;

        cow.stateTimer -= deltaTime;

        // Decide whether to walk or stop grazing
        if (cow.stateTimer <= 0.0f)
        {
            cow.isMoving = !cow.isMoving;

            if (cow.isMoving)
            {
                cow.stateTimer = walkTime(mRng);

                float angle = angleDist(mRng);

                cow.direction = Ogre::Vector3(
                    std::sin(angle),
                    0.0f,
                    std::cos(angle));
            }
            else
            {
                cow.stateTimer = restTime(mRng);
            }
        }

        if (!cow.isMoving)
            continue;

        // Move along the terrain
        Ogre::Vector3 position = cow.node->getPosition();

        position += cow.direction * cow.moveSpeed * deltaTime;

        // Stay inside the generated map
        position.x = Ogre::Math::Clamp(
            position.x, -480.0f, 480.0f);

        position.z = Ogre::Math::Clamp(
            position.z, -480.0f, 480.0f);

        // Match the terrain height
        position.y = getTerrainHeight(position.x, position.z);

        cow.node->setPosition(position);

        // Gradually turn toward the walking direction
        Ogre::Radian targetYaw = Ogre::Math::ATan2(
            cow.direction.x,
            cow.direction.z);

        Ogre::Quaternion targetOrientation(
            targetYaw,
            Ogre::Vector3::UNIT_Y);

        Ogre::Quaternion currentOrientation =
            cow.node->getOrientation();

        float rotationFactor = Ogre::Math::Clamp(
            deltaTime * 3.0f,
            0.0f,
            1.0f);

        cow.node->setOrientation(
            Ogre::Quaternion::Slerp(
                rotationFactor,
                currentOrientation,
                targetOrientation,
                true));
    }
}

void SpawnManager::setUFONode(Ogre::SceneNode *ufoNode)
{
    mUFONode = ufoNode;
}

void SpawnManager::setUFOController(UFOController *controller)
{
    mUFOController = controller;
}

void SpawnManager::updateTanks(float deltaTime)
{
    if (!mUFONode || !mUFOController)
        return;

    const float detectionRange = 150.0f;
    const float detectionDelay = 1.0f;
    const float rotationSpeed = 60.0f;
    const float firingAngle = 5.0f;
    const float firingCooldown = 3.0f;

    Ogre::Vector3 ufoPosition = mUFONode->_getDerivedPosition();

    for (Tank &tank : mTanks)
    {
        if (!tank.node)
            continue;

        Ogre::Vector3 tankPosition =
            tank.node->_getDerivedPosition();

        Ogre::Vector3 direction =
            ufoPosition - tankPosition;

        // Horizontal tracking only
        direction.y = 0.0f;

        float distance = direction.length();

        const float minRange = 20.0f;
        const float maxRange = 100.0f;
        const float maxElevationAngle = 60.0f;

        Ogre::Vector3 toUFO = ufoPosition - tankPosition;

        float horizontalDistance = std::sqrt(
            toUFO.x * toUFO.x +
            toUFO.z * toUFO.z);

        float elevationAngle = Ogre::Math::RadiansToDegrees(
            std::atan2(toUFO.y, horizontalDistance));

        bool inRange =
            horizontalDistance >= minRange &&
            horizontalDistance <= maxRange;

        bool validElevation =
            elevationAngle >= 0.0f &&
            elevationAngle <= maxElevationAngle;

        if (!inRange || !validElevation)
        {
            tank.detectionTimer = 0.0f;
            continue;
        }

        tank.detectionTimer += deltaTime;

        if (tank.detectionTimer < detectionDelay)
            continue;

        if (distance < 0.001f)
            continue;

        direction.normalise();

        // Calculate target orientation
        Ogre::Radian targetYaw =
            Ogre::Math::ATan2(direction.x, direction.z);

        Ogre::Quaternion targetOrientation(
            targetYaw + Ogre::Degree(-90.0f),
            Ogre::Vector3::UNIT_Y);

        Ogre::Quaternion currentOrientation =
            tank.node->getOrientation();

        // Gradually rotate
        float dot = std::abs(
            currentOrientation.Dot(targetOrientation));

        dot = Ogre::Math::Clamp(dot, 0.0f, 1.0f);

        float angle = Ogre::Math::RadiansToDegrees(
            2.0f * std::acos(dot));

        if (angle > 0.001f)
        {
            float rotationFactor = Ogre::Math::Clamp(
                rotationSpeed * deltaTime / angle,
                0.0f,
                1.0f);

            tank.node->setOrientation(
                Ogre::Quaternion::Slerp(
                    rotationFactor,
                    currentOrientation,
                    targetOrientation,
                    true));
        }

        // Recalculate facing angle
        float remainingDot = std::abs(
            tank.node->getOrientation().Dot(targetOrientation));

        remainingDot = Ogre::Math::Clamp(
            remainingDot, 0.0f, 1.0f);

        float remainingAngle = Ogre::Math::RadiansToDegrees(
            2.0f * std::acos(remainingDot));

        tank.fireCooldown -= deltaTime;

        // Fire when facing UFO
        if (remainingAngle <= firingAngle &&
            tank.fireCooldown <= 0.0f)
        {
            createTankMuzzleFlash(tank.node);

            mUFOController->takeDamage(1);

            tank.fireCooldown = firingCooldown;
        }
    }
}

void SpawnManager::createTankMuzzleFlash(Ogre::SceneNode* tankNode)
{
    if (!tankNode || !mParticleManager)
        return;

    // Tank's local muzzle position.
    // The tank model faces local -X.
    Ogre::Vector3 localMuzzleOffset(2.0f, -0.35f, 0.0f);

    // Account for the tank's scale and rotation.
    Ogre::Vector3 scaledOffset(
        localMuzzleOffset.x * tankNode->getScale().x,
        localMuzzleOffset.y * tankNode->getScale().y,
        localMuzzleOffset.z * tankNode->getScale().z
    );

    Ogre::Vector3 worldMuzzlePosition =
        tankNode->_getDerivedPosition() +
        tankNode->_getDerivedOrientation() * scaledOffset;


    mParticleManager->spawnExplosion(
        worldMuzzlePosition,
        "UFO/SmallExplosion",
        0.5f
    );
}

void SpawnManager::setParticleManager(ParticleManager* manager)
{
    mParticleManager = manager;
}