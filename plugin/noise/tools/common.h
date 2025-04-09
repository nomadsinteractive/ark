#pragma once

#include <stdint.h>
#include <list>

#include "noiseutils.h"

namespace ark::plugin::noise {

struct GradientPoint {
    GradientPoint(double pos, const ::noise::utils::Color& color);
    GradientPoint(const GradientPoint& other);

    double _pos;
    ::noise::utils::Color _color;
};

struct CommandLineOptions {
    CommandLineOptions();

    int32_t height(int32_t scale = 1) const;

    const char* _out_file_name;
    int32_t _seed;
    int32_t _octave_count;
    int32_t _width;
    int32_t _height;
    float _frequency;
    float _persistence;
    float _lacunarity;
    char _type;
    char _mod;
    bool _grayscale;
    bool _curl;

    std::list<GradientPoint> _gradient_points;
};

void CreateWhiteNoiseTexture(const int seed, int size, const char* filename);
void CreateTexture(const module::Module& noiseModule, const CommandLineOptions& options);

}

