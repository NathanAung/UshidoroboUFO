#ifndef __PARTICLE_MANAGER_H__
#define __PARTICLE_MANAGER_H__

#include <Ogre.h>
#include <vector>
#include <string>

// Tracks individual active particle systems
struct ActiveParticle {
    Ogre::ParticleSystem* system;
    Ogre::SceneNode* node;
    float timeRemaining;
};

class ParticleManager {
public:
    ParticleManager(Ogre::SceneManager* scnMgr);
    ~ParticleManager();

    // Spawns the explosion at the given location
    void spawnExplosion(const Ogre::Vector3& position);
    
    // Core update loop to prune expired particles
    void update(float deltaTime);

private:
    Ogre::SceneManager* mScnMgr;
    std::vector<ActiveParticle> mActiveParticles;
    int mExplosionCounter; // Ensures unique names for every spawned system
};

#endif