#ifndef __GRAD_H__
#define __GRAD_H__


#include <cmath>
#include <vector>
#include <memory>
#include <array>
#include <algorithm>

class Grad {
public:
    float x, y, z;

    Grad(float x_, float y_, float z_) : x(x_), y(y_), z(z_) {}

    float dot2(float x_, float y_) const {
        return x * x_ + y * y_;
    }

    float dot3(float x_, float y_, float z_) const {
        return x * x_ + y * y_ + z * z_;
    }
};

#endif // __GRAD_H__