#ifndef ARK_APP_IMPL_LAYOUT_LAYOUT_DICTIONARY_H_
#define ARK_APP_IMPL_LAYOUT_LAYOUT_DICTIONARY_H_

#include "core/inf/builder.h"
#include "core/types/shared_ptr.h"

#include "app/forwarding.h"
#include "app/view/view.h"

namespace ark {

class LayoutDictionary {
public:
//  [[plugin::builder::by-value]]
    class DICTIONARY : public Builder<Layout> {
    public:
        DICTIONARY(BeanFactory& factory, const String& value);

        virtual sp<Layout> build(const Scope&) override;

    private:
        String _layout;
        LayoutParam::Gravity _gravity;
    };
};

}

#endif
