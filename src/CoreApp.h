#ifndef __CORE_APP__
#define __CORE_APP__

#include <Ogre.h>
#include <OgreApplicationContext.h>
#include <OgreInput.h>
#include <OgreRTShaderSystem.h>

#include "UFOController.h"

class CoreApp 
    : public OgreBites::ApplicationContext
    , public OgreBites::InputListener
{
public:
    CoreApp();
    ~CoreApp();

    // Setup
    void setup() override;

    // Events
    bool keyPressed(const OgreBites::KeyboardEvent& evt) override;
    bool keyReleased(const OgreBites::KeyboardEvent& evt) override;
    void frameRendered(const Ogre::FrameEvent& evt) override;
    bool mouseMoved(const OgreBites::MouseMotionEvent& evt) override;
    bool mouseWheelRolled(const OgreBites::MouseWheelEvent& evt) override;
    bool mousePressed(const OgreBites::MouseButtonEvent& evt) override;


private:
    Ogre::SceneNode* mainCamera;

    UFOController* ufoController;
};

#endif