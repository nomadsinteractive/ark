#include "test/base/test_case.h"

#include <chrono>
#include <thread>

#include "core/ark.h"
#include "core/base/bean_factory.h"
#include "core/base/clock.h"
#include "core/base/scope.h"
#include "core/base/thread.h"
#include "core/inf/dictionary.h"
#include "core/inf/variable.h"
#include "core/impl/dictionary/dictionary_by_attribute_name.h"
#include "core/traits/expendable.h"
#include "core/types/weak_ptr.h"
#include "core/util/math.h"

#include "graphics/base/transform.h"
#include "graphics/base/quaternion.h"

#include "app/base/application_context.h"
#include "core/base/resource_loader.h"

#include "platform/platform.h"


namespace ark {
namespace unittest {

class FactoriesCase : public TestCase {
public:
    virtual int launch() {
        Ark& ark = Ark::instance();
        WeakPtr<Expendable> we1;

        Scope args;
        const sp<ResourceLoader> resourceLoader = ark.applicationContext()->createResourceLoader("application.xml", args);
        BeanFactory& beanFactory = resourceLoader->beanFactory();
        ark.applicationContext()->sysClock()->setTicker(Platform::getSteadyClock());

        const sp<Expendable> e1 = beanFactory.build<Expendable>("@e1", args);

        TESTCASE_VALIDATE(e1 && !e1->val());

        we1 = e1;

        TESTCASE_VALIDATE(we1.lock());

        std::this_thread::sleep_for(std::chrono::seconds(3));

        TESTCASE_VALIDATE(e1->val());

        sp<Numeric> g1 = beanFactory.build<Numeric>("25.0", args);
        TESTCASE_VALIDATE(g1 && g1->val() == 25.0f);

        sp<Numeric> g2 = beanFactory.build<Numeric>("@linear", args);
        TESTCASE_VALIDATE(g2 && g2->val() == 100.0f);
        sp<Numeric> g3 = beanFactory.build<Numeric>("accelerate", args);
        TESTCASE_VALIDATE(g3 && g3->val() == 110.0f);

        sp<Numeric> g4 = beanFactory.build<Numeric>("@g4", args);
        TESTCASE_VALIDATE(g4 && g4->val() == 0);

        sp<Numeric> g5 = beanFactory.build<Numeric>("@g5", args);
        TESTCASE_VALIDATE(g5 && Math::almostEqual(g5->val(), 1.0f, 4));

        sp<Numeric> g6 = beanFactory.build<Numeric>("@g6", args);
        TESTCASE_VALIDATE(g6 && g6->val() == 5.0f);

        args.put("t", Box(sp<Numeric::Const>::make(12.0f)));
        sp<Numeric> g7 = beanFactory.build<Numeric>("g7", args);
        TESTCASE_VALIDATE(g7 && g7->val() == 12.0f);

        const sp<Vec2> vertex = beanFactory.build<Vec2>("@vertex", args);
        TESTCASE_VALIDATE(vertex && vertex->val().x() == 100.0f && vertex->val().y() == 110.0f);

        Class* clazz = Class::getClass<Vec2>();
        TESTCASE_VALIDATE(clazz);

        const sp<Vec2> v1 = beanFactory.build<Vec2>("@v1", args);
        TESTCASE_VALIDATE(v1 && v1->val().x() == 20 && v1->val().y() == 30);

        const sp<Vec2> v2 = beanFactory.build<Vec2>("v2", args);
        TESTCASE_VALIDATE(v2 && v2->val().x() == 100 && v2->val().y() == 30);

        const sp<Vec2> v3 = beanFactory.build<Vec2>("@v3", args);
        TESTCASE_VALIDATE(v3 && v3->val().x() == 0 && v3->val().y() == 110);

        const sp<Transform> t1 = beanFactory.build<Transform>("t1", args);
        if(!t1 || t1->translation().val().x() != 20.0f || t1->translation().val().y() != 30.0f)
            return 17;

        sp<Transform> t3 = beanFactory.build<Transform>("@t3", args);
        TESTCASE_VALIDATE(t3 && t3->scale().val().x() == 2.0f && t3->scale().val().y() == 2.0f);

        const sp<Expendable> e004 = beanFactory.build<Expendable>("@e004", args);
        const sp<Expendable> e004Copy = beanFactory.build<Expendable>("@e004", args);
        TESTCASE_VALIDATE(e004 == e004Copy);

        const sp<Numeric> g8 = beanFactory.build<Numeric>("@g8", args);
        TESTCASE_VALIDATE(g8 && g8->val() == 8.0f);

        const sp<Numeric> g9 = beanFactory.build<Numeric>("@g9", args);
        TESTCASE_VALIDATE(g9 && g9->val() == 15.0f);

        const sp<Numeric> g10 = beanFactory.build<Numeric>("@g10", args);
        TESTCASE_VALIDATE(g10 && g10->val() == 6.0f);

        const sp<Numeric> g11 = beanFactory.build<Numeric>("@g11", args);
        TESTCASE_VALIDATE(g11 && g11->val() == 14.0f);

        const sp<Numeric> g12 = beanFactory.build<Numeric>("@g12", args);
        TESTCASE_VALIDATE(g12 && g12->val() == 30.0f);

        const sp<Numeric> g13 = beanFactory.build<Numeric>("@g13", args);
        TESTCASE_VALIDATE(g13 && Math::almostEqual(g13->val(), 4.0f, 20));

        const sp<Boolean> g14 = beanFactory.build<Boolean>("@g14", args);
        TESTCASE_VALIDATE(g14 && g14->val() == true);

        const sp<Boolean> g15 = beanFactory.build<Boolean>("@g15", args);
        TESTCASE_VALIDATE(g15 && g15->val() == false);

        const sp<Boolean> g16 = beanFactory.build<Boolean>("@g16", args);
        TESTCASE_VALIDATE(g16 && g16->val() == true);

        const sp<Integer> g17 = beanFactory.build<Integer>("@g17", args);
        TESTCASE_VALIDATE(g17);
        TESTCASE_VALIDATE(g17->val() == 1 && g17->val() == 2 && g17->val() == 3
                          && g17->val() == 1 && g17->val() == 2 && g17->val() == 3);

        const sp<Integer> g18 = beanFactory.build<Integer>("@g18", args);
        TESTCASE_VALIDATE(g18);
        TESTCASE_VALIDATE(g18->val() == 1 && g18->val() == 2 && g18->val() == 3
                          && g18->val() == 2 && g18->val() == 1
                          && g18->val() == 2 && g18->val() == 3);

        const sp<Expendable> disposed = we1.lock();
        TESTCASE_VALIDATE(!disposed);

        return 0;
    }
};

}
}


ark::unittest::TestCase* factories_create() {
    return new ark::unittest::FactoriesCase();
}
