#include "audio.h"

void generateSineWave(int16_t *samples, size_t sampleCount, float frequency, float amplitude, unsigned sampleRate)
{
    const float PI = 3.14159265358979323846f;
    for (size_t i = 0; i < sampleCount; i++)
    {
        float t = (float)i / (float)sampleRate;
        samples[i] = (int16_t)(amplitude * 32767 * sin(2 * PI * frequency * t));
    }
}

sfSoundBuffer *createSineWaveBuffer(float frequency, float amplitude, unsigned sampleRate, float duration)
{
    size_t sampleCount = (size_t)(sampleRate * duration);
    int16_t *samples = malloc(sampleCount * sizeof(int16_t));

    generateSineWave(samples, sampleCount, frequency, amplitude, sampleRate);

    sfSoundBuffer *buffer = sfSoundBuffer_createFromSamples(samples, sampleCount, 1, sampleRate);
    free(samples);

    return buffer;
}

void noise()
{
    float frequency = 440.0f;    // A4 note frequency
    float amplitude = 0.5f;      // Amplitude (0.0 to 1.0)
    unsigned sampleRate = 44100; // Standard sample rate
    float duration = .05f;       // Duration in seconds

    sfSoundBuffer *buffer = createSineWaveBuffer(frequency, amplitude, sampleRate, duration);
    if (!buffer)
    {
        return; // Error handling
    }

    sfSound *sound = sfSound_create();
    sfSound_setBuffer(sound, buffer);
    sfSound_play(sound);

    // Keep the application running while the sound plays
    sfTime durationTime = sfSeconds(duration);
    sfSleep(durationTime);

    // Cleanup
    sfSound_destroy(sound);
    sfSoundBuffer_destroy(buffer);
}