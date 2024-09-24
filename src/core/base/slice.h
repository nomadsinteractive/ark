#pragma once

#include "core/base/api.h"

namespace ark {

class ARK_API Slice {
public:
    Slice(ptrdiff_t begin, ptrdiff_t end, ptrdiff_t step = 1);

    ptrdiff_t begin() const;
    ptrdiff_t end() const;
    ptrdiff_t step() const;

    ptrdiff_t length() const;

    Slice adjustIndices(size_t length) const;

private:
    ptrdiff_t _begin;
    ptrdiff_t _end;
    ptrdiff_t _step;
};

}
