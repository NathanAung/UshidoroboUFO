#ifndef __AUDIO_MANAGER_H__
#define __AUDIO_MANAGER_H__

#include "miniaudio.h"
#include <string>
#include <Ogre.h>
#include <OgreApplicationContext.h>

class AudioManager {
public:
    AudioManager();
    ~AudioManager();

    // Plays a looping background track
    void playBGM(const std::string& filePath);

    void stopBGM();
    
    // Plays a one-shot sound effect (e.g., explosions, abduction)
    void playSFX(const std::string& filePath);

private:
    ma_engine mEngine;
    ma_sound mBgmSound; // Persistent sound object for the background track
    bool mIsInitialized;
    bool mBgmLoaded;    // Tracks if memory has been allocated for the BGM
};

#endif