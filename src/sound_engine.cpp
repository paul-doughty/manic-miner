// sound_engine.cpp
#include "sound_engine.h"
#include <cmath>

SDL_AudioDeviceID SoundEngine::audioDevice = 0;
bool SoundEngine::initialized = false;
bool SoundEngine::playing = false;
std::queue<Note> SoundEngine::noteQueue;
std::mutex SoundEngine::queueMutex;
double SoundEngine::currentTime = 0.0;
double SoundEngine::sampleRate = 44100.0;
Note SoundEngine::currentNote = Note();
double SoundEngine::noteEndTime = 0.0;

bool SoundEngine::initialize() {
    if (SDL_InitSubSystem(SDL_INIT_AUDIO) < 0) {
        return false;
    }

    SDL_AudioSpec want;
    SDL_zero(want);
    want.freq = 44100;
    want.format = AUDIO_S16SYS;
    want.channels = 1;
    want.samples = 1024;
    want.callback = audioCallback;

    audioDevice = SDL_OpenAudioDevice(nullptr, 0, &want, nullptr, 0);
    if (audioDevice == 0) {
        return false;
    }

    initialized = true;
    SDL_PauseAudioDevice(audioDevice, 0);
    return true;
}

double SoundEngine::speccyFreqToHz(uint8_t freq) {
    if (freq == 0) return 0.0;
    // ZX Spectrum used a clock frequency of 3.5MHz
    // and the frequency value was divided by 8 for the timer
    return 3500000.0 / (freq * 8.0 * 16.0);
}

void SoundEngine::audioCallback(void* userdata, Uint8* stream, int len) {
    (void)userdata;
    auto* buffer = (Sint16*)stream;
    int samples = len / sizeof(Sint16);

    const Sint16 SPECTRUM_AMPLITUDE = 8192; // Lower amplitude for more authentic sound

    for (int i = 0; i < samples; i++) {
        Sint16 sample = 0;

        if (currentTime >= noteEndTime) {
            std::lock_guard<std::mutex> lock(queueMutex);
            if (!noteQueue.empty()) {
                currentNote = noteQueue.front();
                noteQueue.pop();
                noteEndTime = currentTime + (currentNote.duration * 0.002); // Adjusted timing
            }
        }

        if (currentTime < noteEndTime) {
            double freq1 = speccyFreqToHz(currentNote.freq1);
            double freq2 = speccyFreqToHz(currentNote.freq2);

            // Generate harsh square waves like the Spectrum
            if (freq1 > 0.0) {
                double t1 = fmod(currentTime * freq1, 1.0);
                sample += (t1 < 0.5 ? SPECTRUM_AMPLITUDE : -SPECTRUM_AMPLITUDE);
            }

            if (freq2 > 0.0) {
                double t2 = fmod(currentTime * freq2, 1.0);
                sample += (t2 < 0.5 ? SPECTRUM_AMPLITUDE : -SPECTRUM_AMPLITUDE);
            }

            // Mix and add distortion characteristic of the Spectrum
            if (freq1 > 0.0 && freq2 > 0.0) {
                sample = (sample > 0 ? SPECTRUM_AMPLITUDE : -SPECTRUM_AMPLITUDE);
            }

            // Add slight noise characteristic of the Spectrum's audio output
            sample += (rand() % 1024) - 512;
        }

        buffer[i] = sample;
        currentTime += 1.0 / sampleRate;
    }

    if (currentTime >= noteEndTime && noteQueue.empty()) {
        playing = false;
    }
}

void SoundEngine::queueNoteWithType(uint8_t freq1, uint8_t freq2, uint8_t duration, SoundType type) {
    if (!initialized) return;

    Note note;  // Create new note
    note.freq1 = freq1;
    note.freq2 = freq2;
    note.duration = duration;
    note.type = type;

    std::lock_guard<std::mutex> lock(queueMutex);
    noteQueue.push(note);
    playing = true;
}


void SoundEngine::playInGameMusic(uint8_t note) {
    queueNoteWithType(note, 0, 90, SoundType::INGAME);  // Increased duration from 10 to 30
}


void SoundEngine::playGameSound(SoundType type) {
    if (!initialized || !playing) return;

    uint8_t freq = 0;
    uint8_t duration = 10;

    switch (type) {
        case SoundType::THEME:
            // Theme music is handled separately
            return;
        case SoundType::INGAME:
            // In-game music is handled separately
            return;
        case SoundType::JUMP:
            freq = JUMP_FREQ;
            duration = 5;
            break;
        case SoundType::COLLECT:
            freq = COLLECT_FREQ;
            duration = 3;
            break;
        case SoundType::DEATH:
            freq = DEATH_FREQ;
            duration = 20;
            break;
        case SoundType::COMPLETE:
            freq = COMPLETE_FREQ;
            duration = 15;
            break;
        default:
            return;
    }

    queueNoteWithType(freq, 0, duration, type);
}


void SoundEngine::clear() {
    std::lock_guard<std::mutex> lock(queueMutex);
    std::queue<Note> empty;
    std::swap(noteQueue, empty);
    playing = false;
    currentTime = 0.0;
    noteEndTime = 0.0;
}