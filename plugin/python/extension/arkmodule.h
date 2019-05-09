#ifndef ARK_PLUGIN_PYTHON_EXTENSION_ARKMODULE_H_
#define ARK_PLUGIN_PYTHON_EXTENSION_ARKMODULE_H_

#include <vector>

#include "core/forwarding.h"


namespace ark {
namespace plugin {
namespace python {

void setPythonPath(const std::vector<String>& paths);

}
}
}

#endif
