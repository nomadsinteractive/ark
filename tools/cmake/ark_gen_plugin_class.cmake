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
    if(MSVC)
        set_source_files_properties(${LOCAL_PLUGIN_FILE_NAME}.cpp PROPERTIES COMPILE_FLAGS "/bigobj")
    endif()

    list(APPEND LOCAL_GENERATED_SRC_LIST ${LOCAL_PLUGIN_FILE_NAME}.h)
    list(APPEND LOCAL_GENERATED_SRC_LIST ${LOCAL_PLUGIN_FILE_NAME}.cpp)
endmacro()

function(ark_gen_plugin_class NAME TYPE NAME_SPACE)
    _ark_gen_plugin_class_impl(${NAME} ${TYPE} "${NAME_SPACE}" ${NAME}_plugin OFF ${ARGN})
    ark_export(LOCAL_GENERATED_SRC_LIST)
endfunction(ark_gen_plugin_class)
