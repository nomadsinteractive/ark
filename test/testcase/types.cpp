#include <random>
#include <vector>

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
        if(!node.as<Node>() || !node.as<A>() || !node.as<D>())
            return 1;
        if(node.as<Node>()->a != 1 || node.as<Node>()->b != 2)
            return 2;
        if(node.as<A>()->a != 1 || node.as<A>()->b != 2)
            return 3;

        sp<A> a = node;
        if(!a.as<Node>() || !a.as<A>() || !a.as<D>())
            return 4;
        if(a.as<Node>()->a != 1 || a.as<Node>()->b != 2)
            return 5;
        if(a->a != 1 || a->b != 2)
            return 6;

        sp<E> e = sp<E>::make();
        a.absorb(e);
        if(!a.is<Node>() || !a.is<A>() || !a.is<D>())
            return 7;
        if(a.as<Node>()->a != 1 || a.as<Node>()->b != 2)
            return 8;
        if(a->a != 1 || a->b != 2)
            return 9;
        if(!e.is<E>())
            return 10;
        if(!a.is<E>() || a.as<E>()->i != 9)
            return 11;

        a.absorb<H>(sp<I>::make());
        if(!a.is<H>())
            return 12;

        sp<D> d = sp<D>::make();
        d.absorb(a);
        if(!d.is<D>() || d.as<D>()->g != 7)
            return 11;
        if(!d.is<Node>() || d.as<Node>()->h != 8)
            return 13;
        if(!d.is<E>() || d.as<E>()->i != 9)
            return 14;

        Class* nodeClass = Class::getClass<Node>();
        if(!nodeClass)
            return 15;

        if(!nodeClass->isInstance(Type<B>::id()))
            return 16;

        const WeakPtr<B> parent = node;
        const sp<B> locked = parent.lock();
        if(!locked || locked->a != 1)
            return 17;

        const sp<Node> n1 = sp<Node>::adopt(new Node());
        const sp<B> castedB = nodeClass->cast(n1.pack(), Type<B>::id()).unpack<B>();
        if(!castedB && castedB->d != 4)
            return 18;

        const sp<D> castedD = nodeClass->cast(n1.pack(), Type<D>::id()).unpack<D>();
        if(!castedD && castedD->h != 8)
            return 19;

        typedef Implementation<uint8_t, uint16_t, uint32_t, uint64_t, Implementation<A, B, D>> implements;
        if(!implements::is<uint8_t>())
            return 20;

        if(!implements::is<uint16_t>() || !implements::is<uint32_t>() || !implements::is<uint64_t>())
            return 21;

        if(implements::is<void>())
            return 22;

        if(!implements::is<A>() || !implements::is<B>() || !implements::is<D>())
            return 23;

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
        if(a2.get() != ap1)
            return 24;
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
            if(desiredLen > buf->length())
                return -4;
            if(dis2(gen))
                _used.insert(buf);
        }

        const Class* renderLayerClass = Class::getClass<RenderLayer>();
        if(!renderLayerClass->isInstance(Type<Renderer>::id()))
            return 25;
        if(String(renderLayerClass->name()) != "RenderLayer")
            return 26;

        const sp<Numeric> duckType = node.as<Numeric>();
        if(!duckType)
            return 27;

        return 0;
    }
};

}
}


ark::unittest::TestCase* types_create() {
    return new ark::unittest::TypesTestCase();
}
