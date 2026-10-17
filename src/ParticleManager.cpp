#include "ParticleManager.h"

ParticleManager::ParticleManager(Ogre::SceneManager* scnMgr)
    : mScnMgr(scnMgr), mExplosionCounter(0)
{
}

ParticleManager::~ParticleManager()
{
    // Ensure all memory is safely cleaned up when the game closes
    for (auto& p : mActiveParticles) {
        p.node->detachAllObjects();
        mScnMgr->destroyParticleSystem(p.system);
        mScnMgr->destroySceneNode(p.node);
    }
    mActiveParticles.clear();
}


void ParticleManager::spawnExplosion(const Ogre::Vector3& position, const std::string& templateName, float lifespan)
{
    std::string sysName = "Explosion_" + std::to_string(mExplosionCounter++);
    
    // Use the passed templateName to create the particle
    Ogre::ParticleSystem* explosion = mScnMgr->createParticleSystem(sysName, templateName);
    
    // Create a standalone node and attach the particle system
    Ogre::SceneNode* explosionNode = mScnMgr->getRootSceneNode()->createChildSceneNode();
    explosionNode->setPosition(position);
    explosionNode->attachObject(explosion);
    
    // Push it to the tracking vector with a dynamic lifespan
    mActiveParticles.push_back({explosion, explosionNode, lifespan});
}


void ParticleManager::update(float deltaTime)
{
    // Iterate backwards to safely remove elements during traversal
    for (int i = mActiveParticles.size() - 1; i >= 0; --i) {
        mActiveParticles[i].timeRemaining -= deltaTime;
        
        if (mActiveParticles[i].timeRemaining <= 0.0f) {
            // Detach and destroy the Ogre objects
            mActiveParticles[i].node->detachAllObjects();
            mScnMgr->destroyParticleSystem(mActiveParticles[i].system);
            mScnMgr->destroySceneNode(mActiveParticles[i].node);
            
            // Remove the struct from our tracking vector
            mActiveParticles.erase(mActiveParticles.begin() + i);
        }
    }
}