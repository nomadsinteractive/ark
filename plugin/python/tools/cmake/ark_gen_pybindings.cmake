macro(ark_gen_pybindings NAME NAMESPACES MODULENAME DIRS)
    unset(LOCAL_GENERATED_SRC_LIST)

    if(MSVC)
        # Some MSVC pragma directives insist to link against the python libraries which we don't need
        ark_find_vcpkg_package(Python3 LIBRARIES Python3::Python COMPONENTS Development MODULE_MODE)
    else()
        ark_find_vcpkg_package(Python3 COMPONENTS Development MODULE_MODE)
    endif()
    list(APPEND LOCAL_SCRIPT_DEPENDS_LIST
        ${ARK_SRC_DIR}/plugin/python/tools/python/gen_core.py
        ${ARK_SRC_DIR}/plugin/python/tools/python/gen_method.py
    )
    ark_add_python_generated_unit(py_${NAME}_bindings ${CMAKE_CURRENT_SOURCE_DIR} "${DIRS}" ${ARK_SRC_DIR}/plugin/python/tools/python/gen_py_bindings.py "${LOCAL_SCRIPT_DEPENDS_LIST}" $<$<CONFIG:RELEASE>:-t> -p ${NAMESPACES} -m ${MODULENAME} -o ${ARK_GENERATED_FILE_DIRECTORY}/py_${NAME}_bindings -l ${ARK_SRC_DIR}/tools/python ${DIRS})
    ark_parse_target_arguments(${ARGN})
    ark_add_plugin_library(ark-${NAME}-pybindings ${LOCAL_SRC_LIST} ${LOCAL_GENERATED_SRC_LIST} ark-python)
endmacro(ark_gen_pybindings)
