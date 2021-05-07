#include "core/base/identifier.h"

#include "test/base/test_case.h"

namespace ark {
namespace unittest {

class IdentifierTestCase : public TestCase {
public:
    virtual int launch() override {

        {
            const Identifier id = Identifier::parse("#horizontal(center)");
            TESTCASE_VALIDATE(id.isVal());
            TESTCASE_VALIDATE(id.val() == "center");
            TESTCASE_VALIDATE(id.valType() == "horizontal");
        }

        {
            const Identifier id = Identifier::parse("@s001");
            TESTCASE_VALIDATE(id.isRef());
            TESTCASE_VALIDATE(id.val() == "");
            TESTCASE_VALIDATE(id.arg() == "");
            TESTCASE_VALIDATE(id.ref() == "s001");
        }

        {
            const Identifier id = Identifier::parse("$args");
            TESTCASE_VALIDATE(id.isArg());
            TESTCASE_VALIDATE(id.ref() == "");
            TESTCASE_VALIDATE(id.val() == "");
            TESTCASE_VALIDATE(id.arg() == "args");
        }

        {
            const Identifier id = Identifier::parse("quad[@app:at001]");
            TESTCASE_VALIDATE(id.isVal());
            TESTCASE_VALIDATE(id.ref() == "");
            TESTCASE_VALIDATE(id.arg() == "");
            TESTCASE_VALIDATE(id.val() == "@app:at001");
            TESTCASE_VALIDATE(id.valType() == "quad");
        }

        {
            const Identifier id = Identifier::parse("(1.0, 1.0, 1.0)");
            TESTCASE_VALIDATE(id.isVal());
            TESTCASE_VALIDATE(id.ref() == "");
            TESTCASE_VALIDATE(id.arg() == "");
            TESTCASE_VALIDATE(id.val() == "(1.0, 1.0, 1.0)");
        }

        return 0;
    }
};

}
}


ark::unittest::TestCase* identifiers_create() {
    return new ark::unittest::IdentifierTestCase();
}
