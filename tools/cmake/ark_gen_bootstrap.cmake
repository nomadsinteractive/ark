macro(ark_gen_bootstrap UNIT_NAME)
    ark_add_python_generated_unit(${UNIT_NAME} ${PROJECT_SOURCE_DIR} "" ${ARK_SRC_DIR}/tools/python/gen_bootstrap.py "" -o ${ARK_GENERATED_FILE_DIRECTORY}/${UNIT_NAME} ${LOCAL_BOOTSTRAP_FUNCS})
endmacro()
