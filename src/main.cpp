// check the plugin cfg before running
// add all include subfolder to cl too

#include <Ogre.h>
#include <OgreApplicationContext.h>
#include <OgreInput.h>
#include <OgreRTShaderSystem.h>

class BasicOgreApp 
    : public OgreBites::ApplicationContext
    , public OgreBites::InputListener
{
public:
    BasicOgreApp() : OgreBites::ApplicationContext("OgreTestApp"), headNode(nullptr), mainCamera(nullptr) {}

    void setup() override {
        OgreBites::ApplicationContext::setup();
        addInputListener(this);

        Ogre::Root* root = getRoot();
        Ogre::SceneManager* scnMgr = root->createSceneManager();

        Ogre::RTShader::ShaderGenerator* shadergen = Ogre::RTShader::ShaderGenerator::getSingletonPtr();
        shadergen->addSceneManager(scnMgr);

        // Camera
        Ogre::Camera* cam = scnMgr->createCamera("MainCam");
        cam->setNearClipDistance(5);
        cam->setAutoAspectRatio(true);
        mainCamera = scnMgr->getRootSceneNode()->createChildSceneNode();
        mainCamera->attachObject(cam);
        mainCamera->setPosition(0, 0, 200);
        getRenderWindow()->addViewport(cam);

        // Light
        Ogre::Light* light = scnMgr->createLight("MainLight");
        Ogre::SceneNode* lightNode = scnMgr->getRootSceneNode()->createChildSceneNode();
        lightNode->setPosition(20, 80, 50);
        lightNode->attachObject(light);
        scnMgr->setSkyBox(true, "Examples/CloudyNoonSkyBox", 5000, true);

        // Ogre head entity
        Ogre::Entity* ogreHead = scnMgr->createEntity("Head", "ogrehead.mesh");
        headNode = scnMgr->getRootSceneNode()->createChildSceneNode();
        headNode->attachObject(ogreHead);

       
    }

    bool keyPressed(const OgreBites::KeyboardEvent& evt) override
    {
        OgreBites::Keycode key = evt.keysym.sym;
        
        if (key == OgreBites::SDLK_ESCAPE) {
            getRoot()->queueEndRendering();
        }

        // keep track of the player's intended direction
        if (key == 'w') headNode->translate(0,1,0);
        else if (key == 'a') headNode->translate(-1,0,0);
        else if (key == 's') headNode->translate(0,-1,0);
        else if (key == 'd') headNode->translate(1,0,0);

        else if (key == OgreBites::SDLK_SPACE)
        {
            
        }

        return true;
    }
  /*
    bool keyReleased(const OgreBites::KeyboardEvent& evt) override
    {
        OgreBites::Keycode key = evt.keysym.sym;
        // keep track of the player's intended direction
        if (key == 'w' ) mKeyDirection.z = 0;
        else if (key == 'a' ) mKeyDirection.x = 0;
        else if (key == 's' ) mKeyDirection.z = 0;
        else if (key == 'd' ) mKeyDirection.x = 0;

        return true;
    } */

    void frameRendered(const Ogre::FrameEvent& evt) override
    {
        headNode->yaw(Ogre::Degree(10.0f * evt.timeSinceLastFrame));
    }

    bool mouseMoved(const OgreBites::MouseMotionEvent& evt) override
    {
        // update camera goal based on mouse movement
         if (mainCamera) {
                mainCamera->yaw(Ogre::Degree(0.05f * evt.xrel));
                mainCamera->pitch(Ogre::Degree(0.05f * evt.yrel));
            }
        return true;
    }

    bool mouseWheelRolled(const OgreBites::MouseWheelEvent& evt) override
    {
        // update camera goal based on mouse movement
        if (headNode) {
                headNode->scale(evt.y *2,evt.y * 2,evt.y * 2);
            }
        return true;
    }

    bool mousePressed(const OgreBites::MouseButtonEvent& evt) override
    {
       
        return true;
    }


private:
    Ogre::SceneNode* headNode;
    Ogre::SceneNode* mainCamera;
};

int main(int argc, char** argv) {
    BasicOgreApp app;
    app.initApp();
    app.getRoot()->startRendering();
    app.closeApp();
    return 0;
}
 