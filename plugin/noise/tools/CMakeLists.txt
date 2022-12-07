project(noise-tool)
cmake_minimum_required(VERSION 2.8)

ark_unset_local_variables()

list(APPEND LOCAL_LIBS libnoise)
list(APPEND LOCAL_LIBS ${LIBPNG_LIBRARIES})

list(APPEND LOCAL_INCLUDE_DIRS .)
list(APPEND LOCAL_INCLUDE_DIRS ../3rdparty/libnoise/include)
list(APPEND LOCAL_INCLUDE_DIRS ${LIBPNG_INCLUDE_DIRS})

aux_source_directory(. LOCAL_SRC_LIST)

ark_add_executable(${PROJECT_NAME} ${LOCAL_SRC_LIST})
