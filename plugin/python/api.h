#pragma once

#include "core/base/api.h"

#ifndef ARK_BUILD_STATIC_PLUGINS
    #ifdef ARK_FLAG_BUILDING_PLUGIN_PYTHON
    #   define ARK_PLUGIN_PYTHON_API ARK_API_EXPORT_DECLARATION
    #else
    #   define ARK_PLUGIN_PYTHON_API ARK_API_IMPORT_DECLARATION
    #endif
#else
#   define ARK_PLUGIN_PYTHON_API
#endif

namespace ark::plugin::python {

#if ARK_FLAG_BUILD_TYPE

void setThreadFlag();
bool getThreadFlag();
void checkThreadFlag();

#define DSET_THREAD_FLAG() setThreadFlag()
#define DGET_THREAD_FLAG() getThreadFlag()
#define DCHECK_THREAD_FLAG() checkThreadFlag()

#else

#define DSET_THREAD_FLAG()
#define DGET_THREAD_FLAG() true
#define DCHECK_THREAD_FLAG()


#endif

}
