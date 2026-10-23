#include "AudioManager.h"
#include <iostream>
#include <filesystem>

#define MEDIA(file) ("../../Media/" file)

AudioManager::AudioManager() : mIsInitialized(false), mBgmLoaded(false)
{
    if (ma_engine_init(NULL, &mEngine) == MA_SUCCESS) {
        mIsInitialized = true;
    } else {
        Ogre::LogManager::getSingleton().logMessage("Failed to initialize miniaudio engine.");
    }
}


AudioManager::~AudioManager()
{
    if (mIsInitialized) {
        // Uninitialize the sound object before shutting down the engine
        if (mBgmLoaded) {
            ma_sound_uninit(&mBgmSound);
        }
        ma_engine_uninit(&mEngine);
    }
}


void AudioManager::playBGM(const std::string& filePath)
{
    if (!mIsInitialized) return;

    // Clean up the previous track if one is already playing
    if (mBgmLoaded) {
        ma_sound_uninit(&mBgmSound);
        mBgmLoaded = false;
    }

    Ogre::String audioPath = MEDIA("audio/" + filePath);
    //Ogre::LogManager::getSingleton().logMessage(audioPath);

    // Initialize the sound. 
    // MA_SOUND_FLAG_STREAM streams the file from disk, saving significant RAM for large music files.
    ma_result result = ma_sound_init_from_file(
        &mEngine, 
        audioPath.c_str(), 
        MA_SOUND_FLAG_STREAM, 
        NULL, 
        NULL, 
        &mBgmSound
    );

    if (result == MA_SUCCESS) {
        mBgmLoaded = true;
        ma_sound_set_looping(&mBgmSound, MA_TRUE); // Tell miniaudio to loop this track infinitely
        ma_sound_start(&mBgmSound);
    } else {
        Ogre::LogManager::getSingleton().logMessage("Failed to load BGM: " + filePath);
    }
}


void AudioManager::stopBGM()
{
    if (!mIsInitialized || !mBgmLoaded) return;
    
    ma_sound_stop(&mBgmSound);
}


void AudioManager::playSFX(const std::string& filePath)
{
    if (!mIsInitialized) return;
    
    Ogre::String audioPath = MEDIA("audio/" + filePath);

    // Short sound effects (lasers, explosions) don't need streaming and can be fired immediately
    ma_engine_play_sound(&mEngine, audioPath.c_str(), NULL);
}