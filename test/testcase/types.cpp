#include <random>
#include <vector>

#include "core/base/string.h"
#include "core/base/object_pool.h"
#include "core/base/memory_pool.h"
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
        TESTCASE_VALIDATE(node.as<Node>() && node.as<A>() && node.as<D>());
        TESTCASE_VALIDATE(node.as<Node>()->a == 1 && node.as<Node>()->b == 2);
        TESTCASE_VALIDATE(node.as<A>()->a == 1 && node.as<A>()->b == 2);
        sp<A> a = node;
        TESTCASE_VALIDATE(a.as<Node>() && a.as<A>() && a.as<D>());
        TESTCASE_VALIDATE(a.as<Node>()->a == 1 && a.as<Node>()->b == 2);
        TESTCASE_VALIDATE(a->a == 1 && a->b == 2);

        sp<E> e = sp<E>::make();
        a.absorb(e);
        TESTCASE_VALIDATE(a.is<Node>() && a.is<A>() && a.is<D>());
        TESTCASE_VALIDATE(a.as<Node>()->a == 1 && a.as<Node>()->b == 2);
        TESTCASE_VALIDATE(a->a == 1 && a->b == 2);
        TESTCASE_VALIDATE(e.is<E>());
        TESTCASE_VALIDATE(a.is<E>() && a.as<E>()->i == 9);

        a.absorb<H>(sp<I>::make());
        TESTCASE_VALIDATE(a.is<H>());

        sp<D> d = sp<D>::make();
        d.absorb(a);
        TESTCASE_VALIDATE(d.is<D>() && d.as<D>()->g == 7);
        TESTCASE_VALIDATE(d.is<Node>() && d.as<Node>()->h == 8);
        TESTCASE_VALIDATE(d.is<E>() && d.as<E>()->i == 9);

        Class* nodeClass = Class::getClass<Node>();
        TESTCASE_VALIDATE(nodeClass);

        TESTCASE_VALIDATE(nodeClass->isInstance(Type<B>::id()));

        const WeakPtr<B> parent = node;
        const sp<B> locked = parent.lock();
        TESTCASE_VALIDATE(locked && locked->a == 1);

        const sp<Node> n1 = sp<Node>::adopt(new Node());
        const sp<B> castedB = nodeClass->cast(n1.pack(), Type<B>::id()).unpack<B>();
        TESTCASE_VALIDATE(castedB && castedB->d == 4);

        const sp<D> castedD = nodeClass->cast(n1.pack(), Type<D>::id()).unpack<D>();
        TESTCASE_VALIDATE(castedD && castedD->h == 8);

        typedef Implementation<uint8_t, uint16_t, uint32_t, uint64_t, Implementation<A, B, D>> implements;
        TESTCASE_VALIDATE(implements::is<uint8_t>());
        TESTCASE_VALIDATE(implements::is<uint16_t>() && implements::is<uint32_t>() && implements::is<uint64_t>());
        TESTCASE_VALIDATE(!implements::is<void>());
        TESTCASE_VALIDATE(implements::is<A>() && implements::is<B>() && implements::is<D>());

        sp<B> nodeB = node;
        Box nb = nodeB.pack();

        sp<D> b = nb.as<D>();

        ObjectPool pool;
        {
            sp<B> p1 = pool.obtain<B>();
            sp<B> p2 = pool.obtain<B>();
            sp<B> p3 = pool.obtain<B>();
            sp<A> p4 = p3;
            sp<A> p5 = p3.cast<A>();
            sp<A> p6 = p5;
        }
        sp<B> p4 = pool.obtain<B>();
        sp<B> p5 = pool.obtain<B>();

        MemoryPool memoryPool(20);
        void* ap1;
        {
            auto a1 = memoryPool.allocate(32);
            ap1 = a1.get();
        }
        auto a2 = memoryPool.allocate(32);
        TESTCASE_VALIDATE(a2.get() == ap1);
        memoryPool.allocate(64);
        memoryPool.allocate(64);
        memoryPool.allocate(19113);
        memoryPool.allocate(20481);

        std::set<array<uint8_t>> _used;
        std::random_device rd;
        std::mt19937 gen(rd());
        std::normal_distribution<> dis1(512 * 1024, 1 << 15);
        std::uniform_int_distribution<> dis2(0, 3);
        for(uint32_t i = 0; i < 1000; i ++) {
            uint32_t desiredLen = static_cast<uint32_t>(dis1(gen));
            const array<uint8_t> buf = memoryPool.allocate(desiredLen);
            TESTCASE_VALIDATE(desiredLen <= buf->length());
            if(dis2(gen))
                _used.insert(buf);
        }

        const Class* renderLayerClass = Class::getClass<RenderLayer>();
        TESTCASE_VALIDATE(renderLayerClass->isInstance(Type<Renderer>::id()));
        TESTCASE_VALIDATE(String(renderLayerClass->name()) == "RenderLayer");

        const sp<Numeric> duckType = node.as<Numeric>();
        TESTCASE_VALIDATE(duckType);

        return 0;
    }
};

}
}


ark::unittest::TestCase* types_create() {
    return new ark::unittest::TypesTestCase();
}
