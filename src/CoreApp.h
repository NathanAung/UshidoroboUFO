#ifndef __CORE_APP__
#define __CORE_APP__

#include <Ogre.h>
#include <OgreApplicationContext.h>
#include <OgreInput.h>
#include <OgreRTShaderSystem.h>
#include <OgreTrays.h>

#include "UFOController.h"
#include "SpawnManager.h"
#include "TerrainController.h"
#include "ParticleManager.h"
#include "AudioManager.h"

class CoreApp 
    : public OgreBites::ApplicationContext
    , public OgreBites::InputListener
    , public OgreBites::TrayListener
{
public:
    CoreApp();
    ~CoreApp();

    // Setup
    void setup() override;
    void shutdown() override;

    // Events
    bool keyPressed(const OgreBites::KeyboardEvent& evt) override;
    bool keyReleased(const OgreBites::KeyboardEvent& evt) override;
    void frameRendered(const Ogre::FrameEvent& evt) override;
    bool mouseMoved(const OgreBites::MouseMotionEvent& evt) override;
    bool mouseWheelRolled(const OgreBites::MouseWheelEvent& evt) override;
    bool mousePressed(const OgreBites::MouseButtonEvent& evt) override;
    bool wantsRestart() const { return restartRequested; }

private:
    void releaseOwnedObjects();

    UFOController* ufoController;
    SpawnManager* spawnManager;
    ParticleManager* mParticleManager;
    AudioManager* mAudioManager;

    OgreBites::TrayManager* trayManager;
    OgreBites::Label* healthLabel;
    OgreBites::Label* cowsLabel;
    OgreBites::Label* timerLabel;
    int displayedHealth;
    int displayedCows;
    int displayedTimerSeconds;
    bool gameEnded = false;
    bool restartRequested = false;
};

#endif