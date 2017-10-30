#include "test/base/test_case.h"

#include "core/base/string_table.h"
#include "core/inf/dictionary.h"
#include "core/types/global.h"

#include "renderer/base/gl_shader.h"
#include "renderer/base/gl_shader_source.h"
#include "renderer/inf/gl_snippet.h"
#include "renderer/util/gl_shader_preprocessor.h"

namespace ark {
namespace unittest {

namespace {

class GLSnippetTest : public GLSnippet {
public:
    virtual void preCompile(GLShaderSource& source, GLShaderPreprocessor::Context& context) override {
        source.addPredefinedAttribute("Alpha01", "float");
        source.addPredefinedAttribute("PointSize", "float");
        source.addUniform("u_Color01", GLUniform::UNIFORM_F4, nullptr, nullptr);
        context.addFragmentColorModifier("vec4(1.0, 1.0, 1.0, v_Alpha01)");
        context.addFragmentColorModifier("u_Color01");
        context.addVertexSource("gl_PointSize = a_PointSize;");

        List<std::pair<String, String>> ins;
        ins.push_back(std::pair<String, String>("vec2", "texCoordinate"));
        context.addFragmentProcedure("shadow", ins, "return c;");
    }
};

}

class GLProgramsTestCase : public TestCase {
public:
    virtual int launch() {
        Global<StringTable> stringTable;
        const sp<String> vert = stringTable->getString("shaders", "texture.vert");
        const sp<String> frag = stringTable->getString("shaders", "texture.frag");
        if(!vert || !frag)
            return -1;

        const sp<GLSnippet> snippet = sp<GLSnippetTest>::make();
        const sp<GLShaderSource> source = sp<GLShaderSource>::make(vert, frag, nullptr);
        source->addSnippet(snippet);
        const GLShader shader(source);

        puts(source->vertex().c_str());
        puts("---------------------------------------------------------------------");
        puts(source->fragment().c_str());
        if(shader.stride() == 0)
            return 1;
        if(!shader.getAttribute("Position").length())
            return 2;
        if(!shader.getAttribute("TexCoordinate").offset())
            return 3;
        if(!shader.getAttribute("Alpha01").offset())
            return 4;

        return 0;
    }
};

}
}

ark::unittest::TestCase* gl_programs_create() {
    return new ark::unittest::GLProgramsTestCase();
}
