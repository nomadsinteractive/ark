#include <string>
#include <map>
#include <vector>

#include "core/inf/array.h"
#include "core/types/null.h"
#include "core/util/strings.h"

#include "graphics/base/rect.h"
#include "graphics/base/color.h"

#include "app/view/view.h"
#include "app/view/layout_param.h"

#include "test/base/test_case.h"

namespace ark {
namespace unittest {

class StringsTestCase : public TestCase {
public:
    virtual int launch() {
        String str = "left:0; top:0; width:20; height:20; auto";
        std::map<String, String> properties = Strings::parseProperties(str);

        TESTCASE_VALIDATE(properties["abc"].empty() && !properties["left"].empty());

        Rect rect = Strings::parse<Rect>("(0, 0, 20, 30)");
        TESTCASE_VALIDATE(rect.left() == 0 && rect.top() == 0 && rect.width() == 20 && rect.height() == 30);

        Rect rectf = Strings::parse<Rect>("0, 0, 20.0, 30.0");
        TESTCASE_VALIDATE(rectf.left() == 0 && rectf.top() == 0 && rectf.width() == 20.0f && rectf.height() == 30.0f);

        Color white = Strings::parse<Color>("#ffffff");
        TESTCASE_VALIDATE(white == Color::WHITE);

        TESTCASE_VALIDATE(Strings::parse<LayoutParam::Gravity>("right") == LayoutParam::GRAVITY_RIGHT);


        const String formatted = Strings::sprintf("hello %s %d", "world", 123);
        TESTCASE_VALIDATE(formatted == "hello world 123");

        const String s001 = "0123456789";
        TESTCASE_VALIDATE(s001.substr(5) == "56789");
        TESTCASE_VALIDATE(s001.substr(5, 7) == "56");
        TESTCASE_VALIDATE(String("").strip() == String(""));
        TESTCASE_VALIDATE(String(" \t").strip() == String(""));
        TESTCASE_VALIDATE(String("a b \t").strip() == String("a b"));
        TESTCASE_VALIDATE(String(" \ra b \t").strip() == String("a b"));
        TESTCASE_VALIDATE(String(" \ra b").strip() == String("a b"));
        const std::regex VAR_PATTERN("\\$\\{?([\\w\\d_]+)\\}?");
        TESTCASE_VALIDATE(String("${a}a = $bb.").replace(VAR_PATTERN,[] (Array<String>& groups) {
                return groups.buf()[1];
            }) == String("aa = bb."));
        return 0;
    }
};

}
}


ark::unittest::TestCase* strings_create() {
    return new ark::unittest::StringsTestCase();
}
