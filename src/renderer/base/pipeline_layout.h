#ifndef ARK_RENDERER_BASE_PIPELINE_LAYOUT_H_
#define ARK_RENDERER_BASE_PIPELINE_LAYOUT_H_

#include <map>

#include "core/base/api.h"
#include "core/base/string.h"
#include "core/collection/list.h"

#include "renderer/forwarding.h"
#include "renderer/base/gl_attribute.h"
#include "renderer/base/gl_pipeline.h"
#include "renderer/base/pipeline_input.h"
#include "renderer/base/gl_shader_preprocessor.h"
#include "renderer/base/gl_uniform.h"
#include "renderer/base/resource_loader_context.h"

namespace ark {

class PipelineLayout {
public:
    PipelineLayout(const sp<RenderController>& renderController, const String& vertex, const String& fragment);

    void loadPredefinedParam(BeanFactory& factory, const sp<Scope>& args, const document& manifest);

    sp<GLProgram> makeGLProgram(GraphicsContext& graphicsContext) const;

    GLAttribute& addPredefinedAttribute(const String& name, const String& type, uint32_t scopes = 0);
    void addUniform(const String& name, GLUniform::Type type, const sp<Flatable>& flatable, const sp<Changed>& changed);

    const sp<GLSnippet>& snippet() const;
    void addSnippet(const sp<GLSnippet>& snippet);

    void preprocess(GraphicsContext& graphicsContext);

    GLShaderPreprocessor& vertex();
    GLShaderPreprocessor& fragment();

    const sp<PipelineInput>& input() const;

private:
    void initialize();

    void addAttribute(const String& name, const String& type);
    void insertPredefinedUniforms(const String& source, StringBuffer& sb);

    void loadPredefinedAttribute(const document& manifest);

    void loadPredefinedUniform(BeanFactory& factory, const sp<Scope>& args, const document& manifest);

private:
    sp<PipelineFactory> _pipeline_factory;

    op<GLShaderPreprocessorContext> _preprocessor_context;

    GLShaderPreprocessor _vertex;
    GLShaderPreprocessor _fragment;

    sp<RenderController> _render_controller;
    sp<PipelineInput> _input;

    sp<GLSnippet> _snippet;

    friend class GLPipeline;
    friend class GLShaderBindings;
    friend class GLShaderPreprocessorContext;

};

}

#endif
