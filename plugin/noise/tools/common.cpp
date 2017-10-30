#include "common.h"

#include <png.h>
#include <cmath>

using namespace noise::utils;

namespace ark {
namespace plugin {
namespace noise {

namespace {
// Given a noise module, this function renders a flat square texture map and
// writes it to a Windows bitmap (*.bmp) file.  Because the texture map is
// square, its width is equal to its height.  The texture map can be seamless
// (tileable) or non-seamless.
void CreatePlanarTexture(const module::Module& noiseModule, const CommandLineOptions& options);

// Given a noise module, this function renders a spherical texture map and
// writes it to a PNG file.  The texture map's width is
// double its height.
void CreateSphericalTexture(const module::Module& noiseModule, const CommandLineOptions& options);
void CreateCylinderTexture(const module::Module& noiseModule, const CommandLineOptions& options);

// Given a noise map, this function renders a texture map and writes it to a
// PNG file.
void RenderTexture(const NoiseMap& noiseMap, const CommandLineOptions& options, int width, int height);
void RenderTextureCURL(const NoiseMap& noiseMap, const CommandLineOptions& options, int width, int height);

void CreatePlanarTexture(const module::Module& noiseModule, const CommandLineOptions& options) {
    // Map the output values from the noise module onto a plane.  This will
    // create a two-dimensional noise map which can be rendered as a flat
    // texture map.
    NoiseMapBuilderPlane plane;
    NoiseMap noiseMap;
    plane.SetBounds(-1.0, 1.0, -1.0, 1.0);
    plane.SetDestSize(options._width, options.height());
    plane.SetSourceModule(noiseModule);
    plane.SetDestNoiseMap(noiseMap);
    plane.EnableSeamless(true);
    plane.Build();

    RenderTexture(noiseMap, options, options._width, options.height());
}

void CreateSphericalTexture(const module::Module& noiseModule, const CommandLineOptions& options) {
    // Map the output values from the noise module onto a sphere.  This will
    // create a two-dimensional noise map which can be rendered as a spherical
    // texture map.
    NoiseMapBuilderSphere sphere;
    NoiseMap noiseMap;
    sphere.SetBounds(-90, 90, -180.0, 180.0);  // degrees
    sphere.SetDestSize(options._width, options.height(2));
    sphere.SetSourceModule(noiseModule);
    sphere.SetDestNoiseMap(noiseMap);
    sphere.Build();

    RenderTexture(noiseMap, options, options._width, options.height(2));
}

void CreateCylinderTexture(const module::Module& noiseModule, const CommandLineOptions& options) {
    // Map the output values from the noise module onto a sphere.  This will
    // create a two-dimensional noise map which can be rendered as a spherical
    // texture map.
    NoiseMapBuilderCylinder cylinder;
    NoiseMap noiseMap;
    cylinder.SetBounds(-180.0, 180.0, -3.1415926 * 4, 3.1415926 * 4);
    cylinder.SetDestSize(options._width, options.height(2));
    cylinder.SetSourceModule(noiseModule);
    cylinder.SetDestNoiseMap(noiseMap);
    cylinder.Build();

    RenderTexture(noiseMap, options, options._width, options.height(2));
}

void RenderTexture(const NoiseMap& noiseMap, const CommandLineOptions& options, int width, int height) {
    if(options._curl) {
        RenderTextureCURL(noiseMap, options, width, height);
        return;
    }

    // Create the color gradients for the texture.
    RendererImage textureRenderer;

    if(!options._grayscale) {
        textureRenderer.ClearGradient ();

        std::list<GradientPoint>::const_iterator iter = options._gradient_points.begin();
        for(; iter != options._gradient_points.end(); ++iter) {
            const GradientPoint& i = *iter;
            textureRenderer.AddGradientPoint(i._pos, i._color);
        }
    }

    // Set up us the texture renderer and pass the noise map to it.
    Image destTexture;
    textureRenderer.SetSourceNoiseMap(noiseMap);
    textureRenderer.SetDestImage(destTexture);

    // Render the texture.
    textureRenderer.Render();

    FILE* fp = fopen(options._out_file_name, "wb");
    if(fp != NULL) {
        png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
        if(png_ptr != NULL) {
            png_infop info_ptr = png_create_info_struct(png_ptr);
            png_init_io(png_ptr, fp);
            png_set_IHDR(png_ptr, info_ptr, width, height,
                         8, options._grayscale ? PNG_COLOR_TYPE_GRAY : PNG_COLOR_TYPE_RGBA, PNG_INTERLACE_NONE,
                         PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
            png_write_info(png_ptr, info_ptr);
            png_bytep row = (png_bytep) malloc((options._grayscale ? 1 : 4) * width * sizeof(png_byte));
            for(int y = 0; y < height; ++y) {
                png_bytep pos_buf = row;
                Color* color = destTexture.GetSlabPtr(y);
                for(int x = 0; x < width; x++) {
                    *pos_buf++ = color->red;
                    if(!options._grayscale) {
                        *pos_buf++ = color->green;
                        *pos_buf++ = color->blue;
                        *pos_buf++ = color->alpha;
                    }
                    color ++;
                }
                png_write_row(png_ptr, row);
            }
            free(row);
            png_free_data(png_ptr, info_ptr, PNG_FREE_ALL, -1);
            png_write_end(png_ptr, NULL);
            png_destroy_write_struct(&png_ptr, NULL);
        }
        fclose(fp);
    }
}

void RenderTextureCURL(const NoiseMap& noiseMap, const CommandLineOptions& options, int width, int height) {
    const float fixedPoint = 2.0f / 65536;

    FILE* fp = fopen(options._out_file_name, "wb");
    if(fp != NULL) {
        png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
        if(png_ptr != NULL) {
            png_infop info_ptr = png_create_info_struct(png_ptr);
            png_init_io(png_ptr, fp);
            png_set_IHDR(png_ptr, info_ptr, width, height,
                         16, PNG_COLOR_TYPE_GA, PNG_INTERLACE_NONE,
                         PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
            png_write_info(png_ptr, info_ptr);
#if !ARK_IS_BIG_ENDIAN
            png_set_swap(png_ptr);
#endif
            png_bytep row = reinterpret_cast<png_bytep>(malloc(2 * width * sizeof(int16_t)));
            for(int y = 0; y < height; ++y) {
                int16_t* pos_buf = reinterpret_cast<int16_t*>(row);
                int16_t dx = 0, dy = 0;
                const float* slabRow = noiseMap.GetConstSlabPtr(y);
                const float* slabNextRow = noiseMap.GetConstSlabPtr(y != height - 1 ? y + 1 : y - 1);
                for(int x = 0; x < width; x++) {
                    float v = std::abs(std::abs(slabRow[x]) - 0.9f) * 10;
                    dx = x != width - 1 ? static_cast<int16_t>((slabRow[x + 1] - slabRow[x]) / fixedPoint * v) : dx;
                    dy = static_cast<int16_t>((y != height - 1 ? slabNextRow[x] - slabRow[x] : slabRow[x] - slabNextRow[x]) / fixedPoint * v);
                    *pos_buf++ = dy;
                    *pos_buf++ = -dx;
                }
                png_write_row(png_ptr, row);
            }
            free(row);
            png_free_data(png_ptr, info_ptr, PNG_FREE_ALL, -1);
            png_write_end(png_ptr, NULL);
            png_destroy_write_struct(&png_ptr, NULL);
        }
        fclose(fp);
    }
}

}

void CreateWhiteNoiseTexture(const int seed, int size, const char* filename) {
    srand(seed);
    FILE* fp = fopen(filename, "wb");
    if(fp != NULL) {
        png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
        if(png_ptr != NULL) {
            int width = size, height = size;
            png_infop info_ptr = png_create_info_struct(png_ptr);
            png_init_io(png_ptr, fp);
            png_set_IHDR(png_ptr, info_ptr, width, height,
                         8, PNG_COLOR_TYPE_RGBA, PNG_INTERLACE_NONE,
                         PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
            png_write_info(png_ptr, info_ptr);
            png_bytep row = (png_bytep) malloc(4 * size * sizeof(png_byte));
            for(int y = 0; y < height; ++y) {
                png_bytep pos_buf = row;
                for(int x = 0; x < width; x++) {
                    *pos_buf++ = static_cast<png_byte>(rand() / ((double) RAND_MAX) * 0xff);
                    *pos_buf++ = static_cast<png_byte>(rand() / ((double) RAND_MAX) * 0xff);
                    *pos_buf++ = static_cast<png_byte>(rand() / ((double) RAND_MAX) * 0xff);
                    *pos_buf++ = 0xff;
                }
                png_write_row(png_ptr, row);
            }
            free(row);
            png_free_data(png_ptr, info_ptr, PNG_FREE_ALL, -1);
            png_write_end(png_ptr, NULL);
            png_destroy_write_struct(&png_ptr, NULL);
        }
        fclose(fp);
    }
}

void CreateTexture(const module::Module& noiseModule, const CommandLineOptions& options)
{
    switch(tolower(options._type)) {
        case 'p':
            CreatePlanarTexture(noiseModule, options);
            break;
        case 'c':
            CreateCylinderTexture(noiseModule, options);
            break;
        case 's':
            CreateSphericalTexture(noiseModule, options);
            break;
    }
}

GradientPoint::GradientPoint(double pos, const Color& color)
    : _pos(pos), _color(color)
{
}

GradientPoint::GradientPoint(const GradientPoint& other)
    : _pos(other._pos), _color(other._color)
{
}

CommandLineOptions::CommandLineOptions()
    : _out_file_name("out.png"), _seed(0), _octave_count(4), _width(512), _height(0), _frequency(2.0f),
      _persistence(0.625f), _lacunarity(2.18359375f), _type('p'), _mod(0), _grayscale(false), _curl(false)
{
}

int32_t CommandLineOptions::height(int32_t scale) const
{
    return _height ? _height : _width / scale;
}

}
}
}
