#ifndef ARK_RENDERER_BASE_PIPELINE_LAYOUT_H_
#define ARK_RENDERER_BASE_PIPELINE_LAYOUT_H_

#include <map>

#include "core/base/api.h"
#include "core/base/string.h"
#include "core/types/owned_ptr.h"

#include "renderer/forwarding.h"
#include "renderer/base/pipeline_input.h"
#include "renderer/base/attribute.h"
#include "renderer/base/gl_shader_preprocessor.h"
#include "renderer/base/uniform.h"
#include "renderer/base/resource_loader_context.h"
#include "renderer/base/shader.h"

namespace ark {

class PipelineLayout {
public:
    PipelineLayout(const sp<RenderController>& renderController, const String& vertex, const String& fragment);

    void loadPredefinedParam(BeanFactory& factory, const sp<Scope>& args, const document& manifest);

    Attribute& addAttribute(const String& name, const String& type, uint32_t scopes = 0);
    void addUniform(const String& name, Uniform::Type type, const sp<Flatable>& flatable, const sp<Changed>& changed);

    const sp<RenderController>& renderController() const;

    const sp<GLSnippet>& snippet() const;
    void addSnippet(const sp<GLSnippet>& snippet);

    void preCompile(GraphicsContext& graphicsContext);

    const sp<PipelineInput>& input() const;
    const GLShaderPreprocessor::Preprocessor& vertex() const;
    const GLShaderPreprocessor::Preprocessor& fragment() const;

private:
    void initialize();

    void insertPredefinedUniforms(const String& source, StringBuffer& sb);

    void loadPredefinedAttribute(const document& manifest);
    void loadPredefinedUniform(BeanFactory& factory, const sp<Scope>& args, const document& manifest);

private:
    sp<RenderController> _render_controller;
    sp<PipelineInput> _input;
    op<PipelineBuildingContext> _preprocessor_context;

    sp<GLSnippet> _snippet;

    GLShaderPreprocessor::Preprocessor _vertex;
    GLShaderPreprocessor::Preprocessor _fragment;

    friend class Shader;
    friend class ShaderBindings;
    friend class PipelineBuildingContext;
    friend class GLShaderPreprocessor;

};

}

#endif