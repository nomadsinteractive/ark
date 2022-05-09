macro(_ark_gen_plugin_class_impl NAME TYPE NAME_SPACE FILE_NAME BUILT_IN)
set(LOCAL_PLUGIN_FILE_NAME ${ARK_GENERATED_FILE_DIRECTORY}/${FILE_NAME})
foreach(i IN LISTS ARGN)
file(GLOB_RECURSE LOCAL_FILES "${i}/*.h")
list(APPEND FILE_DEPENDS ${LOCAL_FILES})
endforeach()
set(PARAM_NAME "-n ${NAME}")
set(PARAM_NAME_SPACE "-s ${NAME_SPACE}")
if(${BUILT_IN})
    set(LOCAL_OPTION_B "-b ${FILE_NAME}")
else()
    set(LOCAL_OPTION_B "")
endif()
add_custom_command(OUTPUT ${LOCAL_PLUGIN_FILE_NAME}.h ${LOCAL_PLUGIN_FILE_NAME}.cpp
    COMMAND ${Python_EXECUTABLE} ${ARK_SRC_DIR}/tools/python/gen_plugin_class.py ${PARAM_NAME} -t ${TYPE} ${PARAM_NAME_SPACE} -o ${LOCAL_PLUGIN_FILE_NAME} ${LOCAL_OPTION_B} ${ARGN}
    DEPENDS ${FILE_DEPENDS} ${ARK_SRC_DIR}/tools/python/gen_plugin_class.py
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
list(APPEND LOCAL_GENERATED_SRC_LIST ${LOCAL_PLUGIN_FILE_NAME}.h)
list(APPEND LOCAL_GENERATED_SRC_LIST ${LOCAL_PLUGIN_FILE_NAME}.cpp)
set(LOCAL_GENERATED_SRC_LIST ${LOCAL_GENERATED_SRC_LIST} PARENT_SCOPE)
endmacro(_ark_gen_plugin_class_impl)

function(ark_gen_plugin_class NAME TYPE NAME_SPACE FILE_NAME)
    _ark_gen_plugin_class_impl(${NAME} ${TYPE} "${NAME_SPACE}" ${FILE_NAME} OFF ${ARGN})
endfunction(ark_gen_plugin_class)


function(ark_gen_builtin_plugin_class NAME NAME_SPACE FILE_NAME)
    _ark_gen_plugin_class_impl("${NAME}" "builtin" "${NAME_SPACE}" ${FILE_NAME} ON ${ARGN})
    list(APPEND LOCAL_BOOTSTRAP_FUNCS "__ark_bootstrap_${FILE_NAME}__")
    set(LOCAL_BOOTSTRAP_FUNCS ${LOCAL_BOOTSTRAP_FUNCS} PARENT_SCOPE)
endfunction(ark_gen_builtin_plugin_class)
