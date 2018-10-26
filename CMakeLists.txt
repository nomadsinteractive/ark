project(ark)
set(ARK_VERSION "0.9.4")
cmake_minimum_required(VERSION 3.0)

include(CMakeDependentOption)

option(ARK_BUILD_SHARED_PLUGINS "Build dynamic link library plugins for ARK runtimes" ON)
option(ARK_BUILD_STATIC_LIBRARY "Build static library" OFF)
CMAKE_DEPENDENT_OPTION(ARK_TEST_ENABLE "Build testcases" ON "ARK_BUILD_STATIC_LIBRARY" OFF)
option(ARK_BUILD_SAMPLES "Build samples" ON)

option(ARK_DOWNLOAD_DEPENDENCIES "Download dependency libraries automatically" ON)
option(ARK_USE_LIBJPEG_TURBO "Use libjpeg-turbo decoding library" ${WIN32})
option(ARK_USE_LIBPNG "Use libpng decoding library" ON)
option(ARK_USE_STB_IMAGE "Use stb image decoding library" ON)

set(ARK_SRC_DIR ${PROJECT_SOURCE_DIR})
set(ARK_TOOLS_DIR ${ARK_SRC_DIR}/tools/cmake)
set(ARK_PLUGIN_DIR ${ARK_SRC_DIR}/plugin)

include(${ARK_TOOLS_DIR}/core.cmake)

set(RUNTIME_SHARED_LIBRARY ${PROJECT_NAME})
set(RUNTIME_STATIC_LIBRARY ${PROJECT_NAME}_static)
set(TESTCASES_EXECUTABLE ${PROJECT_NAME}-testcases)
set(GENERATED_TESTCASE_LAUNCHPAD ${ARK_GENERATED_FILE_DIRECTORY}/testcase_launchpad.cpp)

set(ARK_INCLUDE_DIRS ${ARK_SRC_DIR} ${ARK_SRC_DIR}/src ${ARK_SRC_DIR}/platform/${SYSTEM} ${ARK_SRC_DIR}/platform/any)
set(ARK_LIBRARIES ${RUNTIME_SHARED_LIBRARY})
set(ARK_STATIC_LIBRARIES ${RUNTIME_STATIC_LIBRARY})

list(APPEND LOCAL_INCLUDE_DIRS .)
list(APPEND LOCAL_INCLUDE_DIRS src)

message("Building for ${SYSTEM} - ${CMAKE_BUILD_TYPE}; build-share-libs: ${BUILD_SHARED_LIBS}\nCompiler: ${CMAKE_CXX_COMPILER_ID}-${CMAKE_CXX_COMPILER_VERSION}")

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DARK_FLAG_DEBUG")

ark_test_big_endian()

aux_source_directory(src/core LOCAL_SRC_LIST)
aux_source_directory(src/core/base LOCAL_SRC_LIST)
aux_source_directory(src/core/collection LOCAL_SRC_LIST)
aux_source_directory(src/core/concurrent LOCAL_SRC_LIST)
aux_source_directory(src/core/dom LOCAL_SRC_LIST)
aux_source_directory(src/core/epi LOCAL_SRC_LIST)
aux_source_directory(src/core/impl/asset LOCAL_SRC_LIST)
aux_source_directory(src/core/impl/bean_factory LOCAL_SRC_LIST)
aux_source_directory(src/core/impl/builder LOCAL_SRC_LIST)
aux_source_directory(src/core/impl/dictionary LOCAL_SRC_LIST)
aux_source_directory(src/core/impl/flatable LOCAL_SRC_LIST)
aux_source_directory(src/core/impl/integer LOCAL_SRC_LIST)
aux_source_directory(src/core/impl/message_loop LOCAL_SRC_LIST)
aux_source_directory(src/core/impl/readable LOCAL_SRC_LIST)
aux_source_directory(src/core/impl/runnable LOCAL_SRC_LIST)
aux_source_directory(src/core/impl/boolean LOCAL_SRC_LIST)
aux_source_directory(src/core/impl/numeric LOCAL_SRC_LIST)
aux_source_directory(src/core/impl/variable LOCAL_SRC_LIST)
aux_source_directory(src/core/inf LOCAL_SRC_LIST)
aux_source_directory(src/core/util LOCAL_SRC_LIST)
aux_source_directory(src/core/types LOCAL_SRC_LIST)

aux_source_directory(src/graphics/base LOCAL_SRC_LIST)
aux_source_directory(src/graphics/inf LOCAL_SRC_LIST)
aux_source_directory(src/graphics/impl/alphabet LOCAL_SRC_LIST)
aux_source_directory(src/graphics/impl/bitmap_loader LOCAL_SRC_LIST)
aux_source_directory(src/graphics/impl/boolean LOCAL_SRC_LIST)
aux_source_directory(src/graphics/impl/frame LOCAL_SRC_LIST)
aux_source_directory(src/graphics/impl/flatable LOCAL_SRC_LIST)
aux_source_directory(src/graphics/impl/mat LOCAL_SRC_LIST)
aux_source_directory(src/graphics/impl/renderer LOCAL_SRC_LIST)
aux_source_directory(src/graphics/impl/vec LOCAL_SRC_LIST)
aux_source_directory(src/graphics/util LOCAL_SRC_LIST)

aux_source_directory(src/renderer/base LOCAL_SRC_LIST)
aux_source_directory(src/renderer/impl/atlas_importer LOCAL_SRC_LIST)
aux_source_directory(src/renderer/impl/renderer LOCAL_SRC_LIST)
aux_source_directory(src/renderer/impl/render_command LOCAL_SRC_LIST)
aux_source_directory(src/renderer/impl/gl_model LOCAL_SRC_LIST)
aux_source_directory(src/renderer/impl/gl_snippet LOCAL_SRC_LIST)
aux_source_directory(src/renderer/impl/layer LOCAL_SRC_LIST)
aux_source_directory(src/renderer/impl/render_view LOCAL_SRC_LIST)
aux_source_directory(src/renderer/impl/render_view_factory LOCAL_SRC_LIST)
aux_source_directory(src/renderer/util LOCAL_SRC_LIST)

aux_source_directory(src/renderer/gles20/impl/gl_snippet LOCAL_SRC_LIST)
aux_source_directory(src/renderer/gles20/impl/gl_snippet_factory LOCAL_SRC_LIST)

aux_source_directory(src/renderer/gles30 LOCAL_SRC_LIST)
aux_source_directory(src/renderer/gles30/impl/gl_resource LOCAL_SRC_LIST)
aux_source_directory(src/renderer/gles30/impl/gl_snippet LOCAL_SRC_LIST)
aux_source_directory(src/renderer/gles30/impl/gl_snippet_factory LOCAL_SRC_LIST)
aux_source_directory(src/renderer/gles30/impl/layer LOCAL_SRC_LIST)

aux_source_directory(src/app/base LOCAL_SRC_LIST)
if(NOT ANDROID)
    aux_source_directory(src/app/impl/application LOCAL_SRC_LIST)
endif()
aux_source_directory(src/app/impl/collider LOCAL_SRC_LIST)
aux_source_directory(src/app/impl/event_listener LOCAL_SRC_LIST)
aux_source_directory(src/app/impl/boolean LOCAL_SRC_LIST)
aux_source_directory(src/app/impl/layout LOCAL_SRC_LIST)
aux_source_directory(src/app/impl/renderer LOCAL_SRC_LIST)
aux_source_directory(src/app/impl/tracker LOCAL_SRC_LIST)
aux_source_directory(src/app/impl/vec3 LOCAL_SRC_LIST)
aux_source_directory(src/app/util LOCAL_SRC_LIST)
aux_source_directory(src/app/view LOCAL_SRC_LIST)

aux_source_directory(platform/any LOCAL_SRC_LIST)
ark_include_directories(platform/any)

list(APPEND LOCAL_INCLUDE_DIRS 3rdparty/glm)

ark_add_denpendency(3rdparty/math-neon OFF math-neon 3rdparty/math-neon)
ark_add_denpendency(3rdparty/tinyxml2 OFF tinyxml2 3rdparty/tinyxml2)
ark_add_denpendency(3rdparty/libyaml OFF yaml 3rdparty/libyaml/include)
ark_compile_definitions(-DYAML_DECLARE_STATIC)
ark_add_denpendency(3rdparty/zlib OFF zlibstatic ${CMAKE_CURRENT_BINARY_DIR}/3rdparty/zlib 3rdparty/zlib)

set(ZLIB_ROOT ${ARK_SRC_DIR}/3rdparty/zlib)
set(ZLIB_INCLUDE_DIR ${ARK_SRC_DIR}/3rdparty/zlib ${CMAKE_CURRENT_BINARY_DIR}/3rdparty/zlib)
set(ZLIB_VERSION_STRING "1.2.8")
set(ZLIB_LIBRARY zlibstatic)

ark_add_denpendency(3rdparty/libzip OFF zip ${CMAKE_CURRENT_BINARY_DIR}/3rdparty/libzip 3rdparty/libzip/lib)

set(GLM_TEST_ENABLE OFF)
ark_ensure_dependency(3rdparty/glm)
add_subdirectory(3rdparty/glm)
list(APPEND LOCAL_INCLUDE_DIRS 3rdparty/glm)

ark_ensure_dependency(3rdparty/tinyheaders)
ark_include_directories(3rdparty/tinyheaders)

if(ARK_USE_LIBPNG)
    set(SKIP_INSTALL_ALL ON)
    set(PNG_TESTS OFF)
    set(LIBPNG_INCLUDE_DIR "${ARK_SRC_DIR}/3rdparty/libpng" "${CMAKE_CURRENT_BINARY_DIR}/3rdparty/libpng")
    set(LIBPNG_LIBRARY png_static)
    ark_ensure_dependency(3rdparty/libpng)
    ark_add_denpendency(3rdparty/libpng OFF ${LIBPNG_LIBRARY} ${LIBPNG_INCLUDE_DIR})
    ark_compile_definitions(-DARK_USE_LIBPNG)
endif()

if(ARK_USE_LIBJPEG_TURBO)
    set(ENABLE_SHARED OFF)
    set(WITH_CRT_DLL ON)
    ark_ensure_dependency(3rdparty/libjpeg-turbo)
    ark_add_denpendency(3rdparty/libjpeg-turbo OFF turbojpeg-static 3rdparty/libjpeg-turbo ${CMAKE_CURRENT_BINARY_DIR}/3rdparty/libjpeg-turbo)
    ark_compile_definitions(-DARK_USE_LIBJPEG_TURBO)
endif()

if(ARK_USE_STB_IMAGE)
    ark_ensure_dependency(3rdparty/stb)
    list(APPEND LOCAL_INCLUDE_DIRS 3rdparty/stb)
    ark_compile_definitions(-DARK_USE_STB_IMAGE)
endif()

set(FREETYPE_NO_DIST ON)
ark_add_denpendency(3rdparty/freetype OFF freetype ${CMAKE_CURRENT_BINARY_DIR}/3rdparty/freetype/include/freetype2 3rdparty/freetype/include)

add_subdirectory(plugin/python)
ark_add_export(ark-python ${ARK_PYTHON_LIBRARIES} ${ARK_PYTHON_PATCH_LIBS})
add_subdirectory(plugin/box2d)
ark_add_export(ark-box2d)
add_subdirectory(plugin/vorbis)
add_subdirectory(plugin/portaudio)
add_subdirectory(plugin/noise)

if(NOT ANDROID)
if(${BUILD_SHARED_LIBS})
    set(SDL_SHARED ON)
    set(SDL_STATIC OFF)
    ark_add_denpendency(3rdparty/SDL2 ON SDL2 3rdparty/SDL2/include)
else()
    set(SDL_SHARED OFF)
    set(SDL_STATIC ON)
    add_definitions(-DHAVE_LIBC)
    ark_add_denpendency(3rdparty/SDL2 OFF SDL2-static 3rdparty/SDL2/include)
endif()
endif()

add_subdirectory(plugin/assimp)

if(NOT ANDROID)
add_subdirectory(plugin/pbr)
endif()

ark_setup_tools(ark_gen_string_table)
ark_setup_tools(ark_gen_plugin_class)
ark_setup_tools(ark_gen_bootstrap)
ark_setup_tools(ark_gen_class_hierarchy)

ark_gen_builtin_plugin_class(ark ark framework_plugin src/core src/graphics src/renderer/base src/renderer/impl)
ark_gen_plugin_class(app builtin ark application_plugin src/app)
ark_gen_builtin_plugin_class(gles30 ark::gles30 gles30_plugin src/renderer/gles30)

ark_gen_class_hierarchy(ark ark ${ARK_SRC_DIR}/src core graphics renderer/base renderer/impl app)

ark_gen_string_table(shaders ark::res src/renderer/shaders)
ark_gen_bootstrap(ark_bootstrap)

list(APPEND LOCAL_INCLUDE_DIRS ${CMAKE_CURRENT_BINARY_DIR})

ark_add_shared_library(${RUNTIME_SHARED_LIBRARY} ${LOCAL_SRC_LIST} ${LOCAL_GENERATED_SRC_LIST} ${LOCAL_RUNTIME_LIBRARY_SRC_LIST} ${LOCAL_RES_LIST})
target_compile_definitions(${RUNTIME_SHARED_LIBRARY} PRIVATE -DARK_FLAG_BUILDING_FRAMEWORK)

if(ARK_BUILD_STATIC_LIBRARY)
    ark_add_static_library(${PROJECT_NAME}_static ${LOCAL_SRC_LIST} ${LOCAL_GENERATED_SRC_LIST})
    target_compile_definitions(${PROJECT_NAME}_static PRIVATE -DARK_FLAG_BUILDING_FRAMEWORK -DARK_STATIC)
    if(ARK_TEST_ENABLE)
        aux_source_directory(test/base TEST_SRC_LIST)
        aux_source_directory(test/testcase TEST_SRC_LIST)
        add_custom_command(OUTPUT ${GENERATED_TESTCASE_LAUNCHPAD}
            COMMAND python ${ARK_SRC_DIR}/tools/python/gen_testcase_launchpad.py ${ARK_SRC_DIR}/test/testcase ${GENERATED_TESTCASE_LAUNCHPAD}
            DEPENDS ${TEST_SRC_LIST} ${ARK_SRC_DIR}/tools/python/gen_testcase_launchpad.py
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
        ark_add_executable(${TESTCASES_EXECUTABLE} ${TEST_SRC_LIST} ${GENERATED_TESTCASE_LAUNCHPAD} ${RUNTIME_STATIC_LIBRARY})
        target_compile_definitions(${TESTCASES_EXECUTABLE} PRIVATE -DARK_STATIC)
    endif()
endif()

if(ARK_BUILD_SAMPLES AND NOT ANDROID)
    ark_add_executable(ark-launcher samples/src/main.cpp)
    target_link_libraries(ark-launcher PRIVATE ${ARK_LIBRARIES})
endif()

list(APPEND LOCAL_EXPORT_TARGETS ${RUNTIME_SHARED_LIBRARY})

configure_file(ark-config.cmake.in "${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/${PROJECT_NAME}-config.cmake" @ONLY)

export(TARGETS ${LOCAL_EXPORT_TARGETS} FILE "${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/${PROJECT_NAME}-targets.cmake")

if(WIN32 AND NOT CYGWIN)
  set(DEF_INSTALL_CMAKE_DIR CMake)
else()
  set(DEF_INSTALL_CMAKE_DIR lib/CMake/${PROJECT_NAME})
endif()
set(INSTALL_CMAKE_DIR ${DEF_INSTALL_CMAKE_DIR} CACHE PATH "Installation directory for CMake files")

install(TARGETS ${RUNTIME_SHARED_LIBRARY}
        EXPORT  ark-targets
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib/static)
install(EXPORT ark-targets DESTINATION ${INSTALL_CMAKE_DIR})
