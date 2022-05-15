#ifndef NOISE_H
#define NOISE_H

class Noise
{
public:
    float noise2(float x, float y);
    float noise3(float x, float y, float z);
    float simplex2(
        float x, float y,
        int octaves, float persistence, float lacunarity);

    float simplex3(
        float x, float y, float z,
        int octaves, float persistence, float lacunarity);

};

#endif // NOISE_H
