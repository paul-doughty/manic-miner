#include "globals.h"
#include "helpers.h"
#include "data.h"
#include "music.h"
#include "sound_engine.h"

bool PLAYTUNE() {
    uint8_t freq1;
    uint8_t freq2;
    uint16_t addr;

    SoundEngine::clear();

    for (auto &note : THEMETUNE) {
        const int start_time_ms = getTickCount();

        uint8_t duration = note[0];
        freq1 = note[1];
        addr = PIANOKEY(freq1);
        Speccy_writeMemory(addr, 80);

        freq2 = note[2];
        addr = PIANOKEY(freq2);
        Speccy_writeMemory(addr, 40);

        Speccy_redrawWindow();

        if (game.playMusic) {
            SoundEngine::queueNote(freq1, freq2, duration);
            millisleep(duration * 4);
        } else {
            millisleep(duration * 4);
        }

        if (processInput() == Keyboard::MM_KEY_ENTER) {
            SoundEngine::clear();
            return true;
        }

        addr = PIANOKEY(note[1]);
        Speccy_writeMemory(addr, 56);

        addr = PIANOKEY(note[2]);
        Speccy_writeMemory(addr, 56);

        Speccy_redrawWindow();

        const int elapsed_time_ms = getTickCount() - start_time_ms;
        const int sleep_time = speccy.frameTick - elapsed_time_ms;
        if (sleep_time >= 0) {
            millisleep(sleep_time);
        }
    }

    return false;
}

void playGameMusic() {
    static int noteIndex = 0;
    static int frameCounter = 0;

    if (!game.playMusic) return;

    // Only update the music every 2 frames to slow it down
    frameCounter++;
    if (frameCounter < 2) {
        return;
    }
    frameCounter = 0;

    // Get the current note from GAMETUNE
    uint8_t note = GAMETUNE[noteIndex];

    // Play the note using new method
    SoundEngine::playInGameMusic(note);

    // Move to next note
    noteIndex = (noteIndex + 1) % 64;  // GAMETUNE has 64 notes
}


// Calculate the attribute file address for a piano key.
// Returns with the attribute file address in HL.
// A Frequency parameter from the tune data table at THEMETUNE.
uint16_t PIANOKEY(uint8_t frequency) {
    // Compute the piano key index (K) based on the frequency parameter (F),
    // and store it in bits 0-4 of A: K=31-INT((F-8)/8).
    frequency -= 8;
    frequency = Speccy_rotR(frequency, 3);
    frequency = (uint8_t) ~frequency;

    // A=224+K; this is the LSB.
    frequency |= 224;

    // Set HL to the attribute file address for the piano key.
    return Speccy_buildAddress(89, frequency);
}
