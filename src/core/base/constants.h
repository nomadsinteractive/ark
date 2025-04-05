#pragma once

#include "core/inf/variable.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

namespace ark {

struct Constants {
    Constants();

    document DOCUMENT_NONE;

    sp<Boolean> BOOLEAN_TRUE;
    sp<Boolean> BOOLEAN_FALSE;

    sp<Boundaries> BOUNDARIES_UNIT;

    sp<Model> MODEL_UNIT_QUAD_RHS;
    sp<Model> MODEL_UNIT_QUAD_LHS;
    sp<Model> MODEL_UNIT_NINE_PATCH_TRIANGLE_STRIPS;
    sp<Model> MODEL_UNIT_NINE_PATCH_QUADS;
    sp<Model> MODEL_NDC;

    sp<Numeric> NUMERIC_ZERO;
    sp<Numeric> NUMERIC_ONE;
};

namespace constants {

const char* const ID = "id";
const char* const ALPHA = "alpha";
const char* const ALPHABET = "alphabet";
const char* const ATLAS = "atlas";
const char* const BACKGROUND = "background";
const char* const BINDING = "binding";
const char* const BITMAP = "bitmap";
const char* const BOUNDARY = "boundary";
const char* const BOUNDS = "bounds";
const char* const CAMERA = "camera";
const char* const CLASS = "class";
const char* const CLOCK = "clock";
const char* const COLOR = "color";
const char* const COLLISION_CALLBACK = "collision-callback";
const char* const DELEGATE = "delegate";
const char* const DISCARDED = "discarded";
const char* const ENTITY = "entity";
const char* const EVENT_LISTENER = "event-listener";
const char* const FONT = "font";
const char* const FOREGROUND = "foreground";
const char* const GRAVITY = "gravity";
const char* const LAYOUT = "layout";
const char* const LAYOUT_PARAM = "layout-param";
const char* const LAYER = "layer";
const char* const MODEL_LOADER = "model-loader";
const char* const NAME = "name";
const char* const INTERVAL = "interval";
const char* const POSITION = "position";
const char* const SCALE = "scale";
const char* const SNIPPET = "snippet";
const char* const SRC = "src";
const char* const RECT = "rect";
const char* const WIDTH = "width";
const char* const HEIGHT = "height";
const char* const DEPTH = "depth";
const char* const REF = "ref";
const char* const RENDERER = "renderer";
const char* const RENDER_OBJECT = "render-object";
const char* const RENDER_LAYER = "render-layer";
const char* const RENDER_TARGET = "render-target";
const char* const ROTATION = "rotation";
const char* const SHADER = "shader";
const char* const SIZE = "size";
const char* const TEXT = "text";
const char* const TEXTURE = "texture";
const char* const TIMEOUT = "timeout";
const char* const TRANSFORM = "transform";
const char* const TRANSLATION = "translation";
const char* const TYPE = "type";
const char* const UPLOADER = "uploader";
const char* const VALUE = "value";
const char* const VARYINGS = "varyings";
const char* const VIEW = "view";
const char* const VISIBLE = "visible";

const char* const TOP = "top";
const char* const LEFT = "left";
const char* const BOTTOM = "bottom";
const char* const RIGHT = "right";

const char* const TEXT_COLOR = "text-color";
const char* const TEXT_SIZE = "text-size";

const char* const NINE_PATCH_PADDINGS = "paddings";

constexpr TypeId TYPE_ID_NONE = Type<void>::id();

constexpr std::initializer_list<float> AXIS_X = {1.0f, 0, 0};
constexpr std::initializer_list<float> AXIS_Y = {0, 1.0f, 0};
constexpr std::initializer_list<float> AXIS_Z = {0, 0, 1.0f};

constexpr std::initializer_list<float> SCALE_ONE = {1.0f, 1.0f, 1.0f};
constexpr std::initializer_list<float> QUATERNION_ONE = {0, 0, 0, 1.0f};

}

}
