#include "renderer/impl/gl_snippet/gl_snippet_ucolor.h"

#include "core/util/strings.h"

#include "graphics/impl/flatable/flatable_v4f.h"

#include "renderer/base/gl_shader_source.h"
#include "renderer/base/gl_uniform.h"

namespace ark {

GLSnippetUColor::GLSnippetUColor(const sp<VV4>& color)
    : _color(color)
{
}

void GLSnippetUColor::preInitialize(GLShaderSource& source)
{
    source.addUniform("u_Color", GLUniform::UNIFORM_F4, sp<FlatableV4f>::make(_color), _color.as<Changed>());
}

void GLSnippetUColor::preCompile(GraphicsContext& graphicsContext, GLShaderPreprocessor::Context& context)
{
    context.addFragmentColorModifier("u_Color");
}

GLSnippetUColor::DICTIONARY::DICTIONARY(BeanFactory& factory, const String& value)
    : _color(factory.ensureBuilder<VV4>(value))
{
}

sp<GLSnippet> GLSnippetUColor::DICTIONARY::build(const sp<Scope>& args)
{
    return sp<GLSnippetUColor>::make(_color->build(args));
}

}
