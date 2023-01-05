#include "core/ark.h"
#include "core/base/bean_factory.h"
#include "core/types/null.h"

#include "graphics/base/rect.h"
#include "graphics/base/size.h"

#include "app/inf/layout.h"
#include "app/view/layout_param.h"

#include "test/base/test_case.h"

namespace ark {
namespace unittest {

class LayoutsTestCase : public TestCase {
public:
    LayoutsTestCase()
        : TestCase("layouts.xml"), _client_layout_param(sp<Size>::make(100.0f, 100.0f)), _layout_param(sp<Size>::make(20.0f, 20.0f)) {

    }

    virtual int launch() {
        const sp<BeanFactory> beanFactory = getBeanFactory();
        if(!beanFactory)
            return -1;
        /*
        const Scope args;
        {
            const sp<Layout> layout = beanFactory->build<Layout>("@layout-001", args);
            layout->begin(_client_layout_param);
            Rect r01 = layout->place(_layout_param);
            layout->end();
            if(r01.left() != 40.0f)
                return 1;
            if(r01.top() != 40.0f)
                return 2;
        }
        {
            const sp<Layout> layout = beanFactory->build<Layout>("@layout-002", args);
            layout->begin(_client_layout_param);
            Rect r01 = layout->place(_layout_param);
            layout->end();
            if(r01.left() != 0.0f)
                return 3;
            if(r01.top() != 80.0f)
                return 4;
        }
        {
            const sp<Layout> layout = beanFactory->build<Layout>("@layout-003", args);
            layout->begin(_client_layout_param);
            Rect r01 = layout->place(_layout_param);
            layout->end();
            if(r01.left() != 80.0f)
                return 5;
            if(r01.top() != 0.0f)
                return 6;
        }
        {
            const sp<Layout> layout = beanFactory->build<Layout>("@layout-004", args);
            int r = testLayout(layout, 7, 20.0f, 40.0f, 40.0f, 40.0f, 60.0f, 40.0f);
            if(r != 0)
                return r;
        }
        {
            const sp<Layout> layout = beanFactory->build<Layout>("@layout-005", args);
            int r = testLayout(layout, 13, 0.0f, 0.0f, 20.0f, 0.0f, 40.0f, 0.0f);
            if(r != 0)
                return r;
        }
        {
            const sp<Layout> layout = beanFactory->build<Layout>("@layout-006", args);
            int r = testLayout(layout, 19, 40.0f, 80.0f, 60.0f, 80.0f, 80.0f, 80.0f);
            if(r != 0)
                return r;
        }
        {
            const sp<Layout> layout = beanFactory->build<Layout>("@layout-007", args);
            int r = testLayout(layout, 25, 40.0f, 20.0f, 40.0f, 40.0f, 40.0f, 60.0f);
            if(r != 0)
                return r;
        }
        {
            const sp<Layout> layout = beanFactory->build<Layout>("@layout-008", args);
            int r = testLayout(layout, 31, 0.0f, 40.0f, 0.0f, 60.0f, 0.0f, 80.0f);
            if(r != 0)
                return r;
        }
        {
            const sp<Layout> layout = beanFactory->build<Layout>("@layout-009", args);
            int r = testLayout(layout, 37, 80.0f, 0.0f, 80.0f, 20.0f, 80.0f, 40.0f);
            if(r != 0)
                return r;
        }
        {
            const sp<Layout> layout = beanFactory->build<Layout>("@layout-010", args);
            int r = testLayout(layout, 43, 15.0f, 15.0f, 65.0f, 15.0f, 15.0f, 65.0f);
            if(r != 0)
                return r;
        }
        */
        return 0;
    }

private:
/*
    int testLayout(const sp<Layout>& layout, int base, float l1, float t1, float l2, float t2, float l3, float t3) {
        Layout::Context ctx(_layout_param, []() -> std::vector<sp<LayoutParam>> {
            return {};
        });

        layout->begin(ctx, _client_layout_param);
        Rect r01 = layout->place(ctx, _layout_param);
        Rect r02 = layout->place(ctx, _layout_param);
        Rect r03 = layout->place(ctx, _layout_param);
        const Rect r04 = layout->end(ctx);
        r01.translate(r04.left(), r04.top());
        r02.translate(r04.left(), r04.top());
        r03.translate(r04.left(), r04.top());
        if(r01.left() != l1)
            return base;
        if(r01.top() != t1)
            return base +1;
        if(r02.left() != l2)
            return base + 2;
        if(r02.top() != t2)
            return base + 3;
        if(r03.left() != l3)
            return base + 4;
        if(r03.top() != t3)
            return base + 5;
        return 0;
    }
*/

private:
    LayoutParam _client_layout_param;
    LayoutParam _layout_param;
};

}
}


ark::unittest::TestCase* layouts_create() {
    return new ark::unittest::LayoutsTestCase();
}
