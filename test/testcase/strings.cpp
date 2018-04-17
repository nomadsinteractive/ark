#include <string>
#include <map>
#include <vector>

#include "core/inf/array.h"
#include "core/types/null.h"
#include "core/util/strings.h"

#include "graphics/base/rect.h"
#include "graphics/base/color.h"

#include "app/view/view.h"

#include "test/base/test_case.h"

namespace ark {
namespace unittest {

class StringsTestCase : public TestCase {
public:
    virtual int launch() {
        String str = "left:0; top:0; width:20; height:20; auto";
        const std::map<String, String> properties = Strings::parseProperties(str);
        const String& s = Strings::getProperty(properties, "abc");
        const String& left = Strings::getProperty(properties, "left");
        if(!s.empty() || left.empty())
            return 1;

        Rect rect = Strings::parse<Rect>(str);
        if(rect.left() != 0 || rect.top() != 0 || rect.width() != 20 || rect.height() != 20)
            return 2;

        Rect rectf = Strings::parse<Rect>(str);
        if(rectf.left() != 0 || rectf.top() != 0 || rectf.width() != 20 || rectf.height() != 20)
            return 3;

        rect = Strings::parse<Rect>("(0, 0, 20, 30)");
        if(rect.left() != 0 || rect.top() != 0 || rect.width() != 20 || rect.height() != 30)
            return 4;

        rectf = Strings::parse<Rect>("0, 0, 20.0, 30.0");
        if(rectf.left() != 0 || rectf.top() != 0 || rectf.width() != 20.0f || rectf.height() != 30.0f)
            return 5;

        Color white = Strings::parse<Color>("#ffffff");
        if(white != Color::WHITE)
            return 6;

//        if(Strings::parse<View::Gravity>("?") != View::Gravity::NONE)
//            return 7;

        if(Strings::parse<View::Gravity>("right") != View::Gravity::RIGHT)
            return 8;

        const String formatted = Strings::sprintf("hello %s %d", "world", 123);
        if(formatted != "hello world 123")
            return 9;

        const String s001 = "0123456789";
        if(s001.substr(5) != "56789")
            return 10;
        if(s001.substr(5, 7) != "56")
            return 11;

        if(String("").strip() != String(""))
            return 12;
        if(String(" \t").strip() != String(""))
            return 13;
        if(String("a b \t").strip() != String("a b"))
            return 14;
        if(String(" \ra b \t").strip() != String("a b"))
            return 15;
        if(String(" \ra b").strip() != String("a b"))
            return 16;
        const std::regex VAR_PATTERN("\\$\\{?([\\w\\d_]+)\\}?");
        if(String("${a}a = $bb.").replace(VAR_PATTERN, [] (Array<String>& groups) {
            return groups.buf()[1];
        }) != String("aa = bb."))
            return 17;
        return 0;
    }
};

}
}


ark::unittest::TestCase* strings_create() {
    return new ark::unittest::StringsTestCase();
}
