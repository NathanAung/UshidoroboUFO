#ifndef __UFO_CONTROLLER_H__
#define __UFO_CONTROLLER_H__

#include <Ogre.h>
#include <OgreInput.h>

class UFOController {
public:
    UFOController(Ogre::SceneManager* scnMgr, Ogre::SceneNode* ufoNode);
    ~UFOController();

    // Core update loop for continuous actions
    void update(const Ogre::FrameEvent& evt);

    // Input hooks delegated from CoreApp
    void injectKeyDown(const OgreBites::KeyboardEvent& evt);
    void injectKeyUp(const OgreBites::KeyboardEvent& evt);

private:
    Ogre::SceneManager* mScnMgr;
    Ogre::SceneNode* mUFONode;

    // Movement state flags
    bool mGoingForward;
    bool mGoingBack;
    bool mGoingLeft;
    bool mGoingRight;

    // Base movement speed (units per second)
    float mBaseSpeed;
};

#endif