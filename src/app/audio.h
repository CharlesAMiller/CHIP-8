#ifndef AUDIO_H
#define AUDIO_H

#include <SFML/Audio.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

/**
 * Generates a 
 */
void generateSineWave(int16_t *samples, size_t sampleCount, float frequency, float amplitude, unsigned sampleRate);

/**
 * 
 */
sfSoundBuffer *createSineWaveBuffer(float frequency, float amplitude, unsigned sampleRate, float duration);

/**
 * 
 */
void noise();

#endif