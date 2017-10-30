// texturegranite.cpp
//
// This program uses the libnoise library to generate texture maps consisting
// of granite.
//
// Copyright (C) 2004 Jason Bevins
//
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or (at your option)
// any later version.
//
// This program is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// (COPYING.txt) for more details.
//
// You should have received a copy of the GNU General Public License along
// with this program; if not, write to the Free Software Foundation, Inc., 59
// Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
// The developer's email is jlbezigvins@gmzigail.com (for great email, take
// off every 'zig'.)
//

#include <stdlib.h>

#include <noise/noise.h>
#include <png.h>

#include "common.h"
#include "noiseutils.h"
#include "getopt.h"

using namespace noise;
using namespace ark::plugin::noise;

namespace ark {
namespace plugin {
namespace noise {

int wood(CommandLineOptions& options);
int jade(CommandLineOptions& options);
int slime(CommandLineOptions& options);
int granite(CommandLineOptions& options);

}
}
}

static int sphere(CommandLineOptions& options);

static void usage(const char* file)
{
    printf("%s -t [p]lanar|[c]ylinder|[s]pherical -s seed -o outfile -f frequency -p persistence -l lacunarity -a octavecount -m [j]ade|[g]ranite|[s]lime|[w]ood|[p]erlin -w width -h height -[g]ray -[c]url\n", file);
}

int sphere();

int main(int argc, const char* argv[]) {

    if(argc < 2)
    {
        usage(argv[0]);
        return 0;
    }

    CommandLineOptions options;

    char c;
    while((c = getopt(argc, argv, "s:o:f:p:l:t:m:a:w:h:gc")) != -1) {
        switch(c) {
        case 's':
            options._seed = atoi(optarg);
            break;
        case 'o':
            options._out_file_name = optarg;
            break;
        case 'f':
            options._frequency = static_cast<float>(atof(optarg));
            break;
        case 'p':
            options._persistence = static_cast<float>(atof(optarg));
            break;
        case 'l':
            options._lacunarity = static_cast<float>(atof(optarg));
            break;
        case 'a':
            options._octave_count = atoi(optarg);
            break;
        case 'g':
            options._grayscale = true;
            break;
        case 'c':
            options._curl = true;
            break;
        case 'm':
            options._mod = optarg[0];
            break;
        case 't':
            options._type = optarg[0];
            break;
        case 'w':
            options._width = atoi(optarg);
            break;
        case 'h':
            options._height = atoi(optarg);
            break;
        }
    }

    if(!options._mod)
        return 0;

    switch(options._mod) {
    case 'n':
        CreateWhiteNoiseTexture(options._seed, options._width, options._out_file_name);
        break;
    case 'w':
        wood(options);
        break;
    case 'j':
        jade(options);
        break;
    case 's':
        slime(options);
        break;
    case 'g':
        granite(options);
        break;
    case 'p':
        sphere(options);
        break;
    }
    return 0;
}

int sphere(CommandLineOptions& options)
{
    module::Perlin myModule;

    options._gradient_points.push_back(GradientPoint(-1.0000, utils::Color (  0,   0, 128, 255)));
    options._gradient_points.push_back(GradientPoint(-0.2500, utils::Color (  0,   0, 255, 255)));
    options._gradient_points.push_back(GradientPoint(0.0000, utils::Color (  0, 128, 255, 255)));
    options._gradient_points.push_back(GradientPoint(0.0625, utils::Color (240, 240,  64, 255)));
    options._gradient_points.push_back(GradientPoint(0.1250, utils::Color ( 32, 160,   0, 255)));
    options._gradient_points.push_back(GradientPoint(0.3750, utils::Color (224, 224,   0, 255)));
    options._gradient_points.push_back(GradientPoint(0.7500, utils::Color (128, 128, 128, 255)));
    options._gradient_points.push_back(GradientPoint(1.0000, utils::Color (255, 255, 255, 255)));


    CreateTexture(myModule, options);

    return 0;
}
