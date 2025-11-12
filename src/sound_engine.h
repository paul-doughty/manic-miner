// sound_engine.h
#pragma once
#include <SDL.h>
#include <queue>
#include <mutex>

enum class SoundType {
    THEME,
    INGAME,
    JUMP,
    COLLECT,
    DEATH,
    COMPLETE
};

struct Note {
    uint8_t freq1;
    uint8_t freq2;
    uint8_t duration;
    SoundType type;

    Note() : freq1(0), freq2(0), duration(0), type(SoundType::THEME) {}  // Default constructor
};



class SoundEngine {
public:
    static bool initialize();
    static void shutdown();
    // Keep original signature for compatibility
    static void queueNote(uint8_t freq1, uint8_t freq2, uint8_t duration) {
        queueNoteWithType(freq1, freq2, duration, SoundType::THEME);
    }
    static void clear();
    static bool isPlaying() { return playing; }

    // Add new methods for game sounds
    static void playGameSound(SoundType type);
    static void playInGameMusic(uint8_t note);

private:
    static void queueNoteWithType(uint8_t freq1, uint8_t freq2, uint8_t duration, SoundType type);
    static void audioCallback(void* userdata, Uint8* stream, int len);
    static double speccyFreqToHz(uint8_t freq);

    static const uint8_t JUMP_FREQ = 150;
    static const uint8_t COLLECT_FREQ = 200;
    static const uint8_t DEATH_FREQ = 50;
    static const uint8_t COMPLETE_FREQ = 100;

    static SDL_AudioDeviceID audioDevice;
    static bool initialized;
    static bool playing;
    static std::queue<Note> noteQueue;
    static std::mutex queueMutex;
    static double currentTime;
    static double sampleRate;
    static Note currentNote;
    static double noteEndTime;
};