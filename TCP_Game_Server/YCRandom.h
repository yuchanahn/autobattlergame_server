#pragma once
#include <iostream>
#include <random>

using namespace std;

namespace yc {
    static int rand(int min, int max)
    {
        random_device rn;
        mt19937_64 rnd(rn());
        uniform_int_distribution<int> range(min, max);
        return range(rnd);
    }
    static float rand(float min, float max)
    {
        random_device rn;
        mt19937 rnd(rn());
        uniform_real_distribution<> range(min, max);
        return static_cast<float>(range(rnd));
    }

    template <typename T>
    static T random_dir2d()
    {
        return T(cos(rand(1.f, 360.f)), sin(rand(1.f, 360.f))).normalize();
    }
    template <typename T>
    static T random_pos(float min, float max)
    {
        return T(yc::rand(min, max), yc::rand(min, max));
    }

    static float random_unit_curcle()
    {
        //cos(rand() % 360);
        //sin(rand() % 360);
    }
}