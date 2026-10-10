#ifndef __NOISE_H__
#define __NOISE_H__

#include <cmath>
#include <vector>
#include <memory>
#include <array>
#include <algorithm>
#include <iostream>
#include "Grad.h"

class Noise {
private:
    std::vector<int> perm;
    std::vector<std::shared_ptr<Grad>> gradP;
    std::vector<std::shared_ptr<Grad>> grad3;
    std::vector<int> p;

public:
    Noise();
    ~Noise();

    void seed(int seed);
    float perlin2(float x, float y);
    static float normalize(float value, float max, float min);
    static float clamp(float value, float low, float high);

private:
    float fade(float t) const {
        return t * t * t * (t * (t * 6 - 15) + 10);
    }

    float lerp(float a, float b, float t) const {
        return a + t * (b - a);
    }

    
};

#endif // __NOISE_H__