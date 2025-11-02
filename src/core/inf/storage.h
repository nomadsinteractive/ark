#pragma once

#include "core/forwarding.h"

#include "core/inf/importer.h"
#include "core/inf/outputer.h"

namespace ark {

class Storage {
public:
    virtual ~Storage() = default;

    virtual void import(const sp<Readable>& src) = 0;
    virtual void output(const sp<Writable>& out) = 0;

    virtual void jsonLoad(const Json& json) = 0;
    virtual Json jsonDump() = 0;

    template<typename T> class Composite;
};

template<typename T> class Storage::Composite : public Storage {
public:
    Composite(T& obj, sp<Importer<T>> importer, sp<Outputer<T>> outputer)
        : _obj(obj), _importer(std::move(importer)), _outputer(std::move(outputer)) {
    }

    void import(const sp<Readable>& src) override {
        DASSERT(_importer);
        _importer->import(_obj, src);
    }

    void output(const sp<Writable>& out) override {
        DASSERT(_outputer);
        _outputer->output(_obj, out);
    }

protected:
    T& _obj;
    sp<Importer<T>> _importer;
    sp<Outputer<T>> _outputer;
};

}
