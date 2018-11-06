#ifndef ARK_GRAPHICS_FORWARDING_H_
#define ARK_GRAPHICS_FORWARDING_H_

#include "core/forwarding.h"

namespace ark {

class Alphabet;
class Bitmap;
class Block;
class Bounds;
class Camera;
class Color;
class Font;
class Frame;
class ImageResource;
class Scrollable;
class Layer;
class Matrix;
class Mat3Impl;
class Model;
class Layer;
class LayerContext;
class Varyings;
class TileMaker;
class Transform;
class Transformable;
class Rect;
class RenderCommand;
class RenderCommandPipeline;
class RenderObject;
class RenderView;
class RenderLayer;
class RenderRequest;
class Renderer;
class RendererGroup;
class Rotation;
class Size;
class SurfaceController;
class Vec2Impl;
class Vec3Impl;
class Vec4Impl;
class V2;
class V3;
class V4;
class Vec2Util;
class Viewport;

template<uint32_t S> class Mat;

typedef sp<Bitmap> bitmap;

typedef Mat<3> M3;
typedef Mat<4> M4;

typedef Variable<M3> Mat3;
typedef Variable<M4> Mat4;

typedef Variable<V2> Vec2;
typedef Variable<V3> Vec3;
typedef Variable<V4> Vec4;

typedef Loader<Bitmap> BitmapLoader;

typedef Vec2 Vec;
typedef V2 V;
typedef Vec2Impl VecImpl;
typedef Vec2Util VecUtil;

const int32_t DIMENSIONS = 2;

}

#endif
