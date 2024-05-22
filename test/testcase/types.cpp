#include <random>
#include <vector>

#include "core/base/string.h"
#include "core/inf/array.h"
#include "core/inf/duck.h"
#include "core/inf/variable.h"
#include "core/types/class.h"
#include "core/types/shared_ptr.h"
#include "core/types/weak_ptr.h"

#include "graphics/forwarding.h"

#include "test/base/test_case.h"

class A {
public:
    virtual ~A() = default;
    int a = 1;
    int b = 2;
    int c = 3;
};

class B : public A, public ark::Implements<B, A> {
public:
    virtual ~B() = default;
    int d = 4;
    int e = 5;
    int f = 6;
};

class D {
public:
    virtual ~D() = default;
    int g = 7;
    int h = 8;
};

class E {
public:
    int i = 9;
};

class H {
public:
    virtual ~H() = default;

    int j = 10;
};

class I : public H, ark::Implements<I, H> {
public:
    int k = 11;
};

class Node final : public B, public D, public ark::Duck<ark::Numeric>, ark::Implements<Node, A, B, D, ark::Duck<ark::Numeric>> {
public:
    Node() {
    }

    Node(const Node&) {
    }

    virtual void to(ark::sp<ark::Numeric>& inst) override {
        inst = ark::sp<ark::Numeric::Impl>::make(0.0f);
    }
};

namespace ark {
namespace unittest {

class TypesTestCase : public TestCase {
public:
    virtual int launch() {

#ifdef ARK_USE_CONSTEXPR
        static_assert(Type<B>::id(), "Compile time constexpr failed");
#endif

        const sp<B> node = sp<Node>::make();
        TESTCASE_VALIDATE(node.tryCast<Node>() && node.tryCast<A>() && node.tryCast<D>());
        TESTCASE_VALIDATE(node.tryCast<Node>()->a == 1 && node.tryCast<Node>()->b == 2);
        TESTCASE_VALIDATE(node.tryCast<A>()->a == 1 && node.tryCast<A>()->b == 2);
        sp<A> a = node;
        TESTCASE_VALIDATE(a.tryCast<Node>() && a.tryCast<A>() && a.tryCast<D>());
        TESTCASE_VALIDATE(a.tryCast<Node>()->a == 1 && a.tryCast<Node>()->b == 2);
        TESTCASE_VALIDATE(a->a == 1 && a->b == 2);

        sp<E> e = sp<E>::make();
//        a.absorb(e);
        TESTCASE_VALIDATE(a.isInstance<Node>() && a.isInstance<A>() && a.isInstance<D>());
        TESTCASE_VALIDATE(a.tryCast<Node>()->a == 1 && a.tryCast<Node>()->b == 2);
        TESTCASE_VALIDATE(a->a == 1 && a->b == 2);
        TESTCASE_VALIDATE(e.isInstance<E>());
        TESTCASE_VALIDATE(a.isInstance<E>() && a.tryCast<E>()->i == 9);

//        a.absorb<H>(sp<I>::make());
        TESTCASE_VALIDATE(a.isInstance<H>());

        sp<D> d = sp<D>::make();
//        d.absorb(a);
        TESTCASE_VALIDATE(d.isInstance<D>() && d.tryCast<D>()->g == 7);
        TESTCASE_VALIDATE(d.isInstance<Node>() && d.tryCast<Node>()->h == 8);
        TESTCASE_VALIDATE(d.isInstance<E>() && d.tryCast<E>()->i == 9);

        Class* nodeClass = Class::getClass<Node>();
        TESTCASE_VALIDATE(nodeClass);

        TESTCASE_VALIDATE(nodeClass->isInstance(Type<B>::id()));

        const WeakPtr<B> parent = node;
        const sp<B> locked = parent.lock();
        TESTCASE_VALIDATE(locked && locked->a == 1);

        const sp<Node> n1 = sp<Node>::adopt(new Node());
        const sp<B> castedB = nodeClass->cast(n1, Type<B>::id()).toPtr<B>();
        TESTCASE_VALIDATE(castedB && castedB->d == 4);

        const sp<D> castedD = nodeClass->cast(n1, Type<D>::id()).toPtr<D>();
        TESTCASE_VALIDATE(castedD && castedD->h == 8);

        sp<B> nodeB = node;
        Box nb = nodeB;

        sp<D> b = nb.as<D>();

        const Class* renderLayerClass = Class::getClass<RenderLayer>();
        TESTCASE_VALIDATE(renderLayerClass->isInstance(Type<Renderer>::id()));
        TESTCASE_VALIDATE(String(renderLayerClass->name()) == "RenderLayer");

        const sp<Numeric> duckType = node.tryCast<Numeric>();
        TESTCASE_VALIDATE(duckType);

        return 0;
    }
};

}
}


ark::unittest::TestCase* types_create() {
    return new ark::unittest::TypesTestCase();
}
