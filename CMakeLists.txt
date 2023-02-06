cmake_minimum_required(VERSION 3.15)

project(ark VERSION 0.9.5)

include(CMakeDependentOption)

set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build shared libs")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(ARK_SRC_DIR ${PROJECT_SOURCE_DIR})
set(ARK_TOOLS_DIR ${ARK_SRC_DIR}/tools/cmake)
set(ARK_3RDPARTY_DIR ${ARK_SRC_DIR}/3rdparty)
set(ARK_PLUGIN_DIR ${ARK_SRC_DIR}/plugin)

set(ARK_RUNTIME_SHARED_LIBRARY ${PROJECT_NAME})
set(ARK_RUNTIME_STATIC_LIBRARY ${PROJECT_NAME}_static)
set(TESTCASES_EXECUTABLE ${PROJECT_NAME}-testcases)

set(ARK_INCLUDE_DIRS ${ARK_SRC_DIR} ${ARK_SRC_DIR}/src ${ARK_SRC_DIR}/platform/${SYSTEM} ${ARK_SRC_DIR}/platform/any)
set(ARK_LIBRARIES ${ARK_RUNTIME_SHARED_LIBRARY})
set(ARK_STATIC_LIBRARIES ${ARK_RUNTIME_STATIC_LIBRARY})

include(${ARK_TOOLS_DIR}/core.cmake)

ark_find_vcpkg_package(ZLIB LIBRARIES ZLIB::ZLIB MODULE_MODE)

find_package(Python COMPONENTS Interpreter)

set(ARK_SYSTEM_NAME ${SYSTEM} CACHE STRING "Target system name of ARK was built for")

ark_add_resource_dir(src platform plugin tools test samples)
ark_add_resource_file(ark-config.cmake.in README.md LICENSE vcpkg.json)

option(ARK_BUILD_STATIC_LIBRARY "Build static library" OFF)
option(ARK_BUILD_SAMPLES "Build samples" ON)
ark_compile_definition_option(ARK_BUILD_TESTS "Build testcases" ON)

option(ARK_BUILD_PLUGIN_FMOD "Build fmod plugin" OFF)

option(ARK_DOWNLOAD_DEPENDENCIES "Download dependency libraries automatically" ON)

ark_compile_definition_option(ARK_BUILD_STATIC_PLUGINS "Build static plugin libraries" OFF)
if(WIN32)
    ark_compile_definition_option(ARK_USE_LIBJPEG_TURBO "Use libjpeg-turbo decoding library" OFF)
endif()

ark_compile_definition_option(ARK_USE_CONSTEXPR  "Use constexpr wherever possible" ON)
ark_compile_definition_option(ARK_USE_LIBPNG "Use libpng decoding library" ON)
ark_compile_definition_option(ARK_USE_STB_IMAGE "Use stb image decoding library" ON)

ark_compile_definition_option(ARK_USE_OPEN_GL "Use OpenGL rendering library" ON)
ark_compile_definition_option(ARK_USE_VULKAN  "Use Vulkan rendering library" OFF)

ark_compile_definition_option(ARK_FLAG_PUBLISHING_BUILD "This is a release build for publishing. Disabling all the unnecessary modules" OFF)

list(APPEND LOCAL_INCLUDE_DIRS .)
list(APPEND LOCAL_INCLUDE_DIRS src)

message("Building for ${ARK_SYSTEM_NAME} - ${CMAKE_BUILD_TYPE}; build-share-libs: ${BUILD_SHARED_LIBS}\nCompiler: ${CMAKE_CXX_COMPILER_ID}-${CMAKE_CXX_COMPILER_VERSION}")
ark_compile_definitions(-DARK_SYSTEM_NAME="${ARK_SYSTEM_NAME}")
ark_compile_definitions(-DARK_BINARY_POSTFIX="$<$<CONFIG:DEBUG>:${CMAKE_DEBUG_POSTFIX}>$<$<CONFIG:RELWITHDEBINFO>:${CMAKE_RELWITHDEBINFO_POSTFIX}>")

ark_add_debug_flag()

ark_test_big_endian()

aux_source_directory(src/core LOCAL_SRC_LIST)
aux_source_directory(src/core/base LOCAL_SRC_LIST)
aux_source_directory(src/core/collection LOCAL_SRC_LIST)
aux_source_directory(src/core/concurrent LOCAL_SRC_LIST)
aux_source_directory(src/core/dom LOCAL_SRC_LIST)
aux_source_directory(src/core/epi LOCAL_SRC_LIST)
aux_source_directory(src/core/impl/asset LOCAL_SRC_LIST)
aux_source_directory(src/core/impl/asset_bundle LOCAL_SRC_LIST)
aux_source_directory(src/core/impl/bean_factory LOCAL_SRC_LIST)
aux_source_directory(src/core/impl/boolean LOCAL_SRC_LIST)
aux_source_directory(src/core/impl/dictionary LOCAL_SRC_LIST)
aux_source_directory(src/core/impl/executor LOCAL_SRC_LIST)
aux_source_directory(src/core/impl/input LOCAL_SRC_LIST)
aux_source_directory(src/core/impl/importer LOCAL_SRC_LIST)
aux_source_directory(src/core/impl/integer LOCAL_SRC_LIST)
aux_source_directory(src/core/impl/loader LOCAL_SRC_LIST)
aux_source_directory(src/core/impl/message_loop LOCAL_SRC_LIST)
aux_source_directory(src/core/impl/numeric LOCAL_SRC_LIST)
aux_source_directory(src/core/impl/readable LOCAL_SRC_LIST)
aux_source_directory(src/core/impl/runnable LOCAL_SRC_LIST)
aux_source_directory(src/core/impl/string_bundle LOCAL_SRC_LIST)
aux_source_directory(src/core/impl/updatable LOCAL_SRC_LIST)
aux_source_directory(src/core/impl/variable LOCAL_SRC_LIST)
aux_source_directory(src/core/impl/writable LOCAL_SRC_LIST)
aux_source_directory(src/core/util LOCAL_SRC_LIST)
aux_source_directory(src/core/types LOCAL_SRC_LIST)

aux_source_directory(src/graphics/base LOCAL_SRC_LIST)
aux_source_directory(src/graphics/inf LOCAL_SRC_LIST)
aux_source_directory(src/graphics/impl/alphabet LOCAL_SRC_LIST)
aux_source_directory(src/graphics/impl/bitmap_loader LOCAL_SRC_LIST)
aux_source_directory(src/graphics/impl/boolean LOCAL_SRC_LIST)
aux_source_directory(src/graphics/impl/glyph_maker LOCAL_SRC_LIST)
aux_source_directory(src/graphics/impl/frame LOCAL_SRC_LIST)
aux_source_directory(src/graphics/impl/input LOCAL_SRC_LIST)
aux_source_directory(src/graphics/impl/mat LOCAL_SRC_LIST)
aux_source_directory(src/graphics/impl/render_batch LOCAL_SRC_LIST)
aux_source_directory(src/graphics/impl/renderable LOCAL_SRC_LIST)
aux_source_directory(src/graphics/impl/renderer LOCAL_SRC_LIST)
aux_source_directory(src/graphics/impl/storage LOCAL_SRC_LIST)
aux_source_directory(src/graphics/impl/transform LOCAL_SRC_LIST)
aux_source_directory(src/graphics/impl/vec LOCAL_SRC_LIST)
aux_source_directory(src/graphics/util LOCAL_SRC_LIST)

aux_source_directory(src/renderer/base LOCAL_SRC_LIST)
aux_source_directory(src/renderer/impl/importer LOCAL_SRC_LIST)
aux_source_directory(src/renderer/impl/layer LOCAL_SRC_LIST)
aux_source_directory(src/renderer/impl/model_loader LOCAL_SRC_LIST)
aux_source_directory(src/renderer/impl/renderer LOCAL_SRC_LIST)
aux_source_directory(src/renderer/impl/render_command_composer LOCAL_SRC_LIST)
aux_source_directory(src/renderer/impl/vertices LOCAL_SRC_LIST)
aux_source_directory(src/renderer/impl/snippet LOCAL_SRC_LIST)
aux_source_directory(src/renderer/inf LOCAL_SRC_LIST)
aux_source_directory(src/renderer/util LOCAL_SRC_LIST)

if(ARK_USE_OPEN_GL)
    aux_source_directory(src/renderer/opengl/base LOCAL_SRC_LIST)
    aux_source_directory(src/renderer/opengl/pipeline_factory LOCAL_SRC_LIST)
    aux_source_directory(src/renderer/opengl/es20/snippet_factory LOCAL_SRC_LIST)
    aux_source_directory(src/renderer/opengl/es30/gl_resource LOCAL_SRC_LIST)
    aux_source_directory(src/renderer/opengl/es30/snippet_factory LOCAL_SRC_LIST)
    aux_source_directory(src/renderer/opengl/renderer LOCAL_SRC_LIST)
    aux_source_directory(src/renderer/opengl/render_view LOCAL_SRC_LIST)
    aux_source_directory(src/renderer/opengl/renderer_factory LOCAL_SRC_LIST)
    aux_source_directory(src/renderer/opengl/snippet LOCAL_SRC_LIST)
    aux_source_directory(src/renderer/opengl/util LOCAL_SRC_LIST)
endif()

aux_source_directory(src/app/base LOCAL_SRC_LIST)
if(NOT (ANDROID OR IOS))
    aux_source_directory(src/app/impl/application LOCAL_SRC_LIST)
endif()
aux_source_directory(src/app/impl/boolean LOCAL_SRC_LIST)
aux_source_directory(src/app/impl/broad_phrase LOCAL_SRC_LIST)
aux_source_directory(src/app/impl/collider LOCAL_SRC_LIST)
aux_source_directory(src/app/impl/event_listener LOCAL_SRC_LIST)
aux_source_directory(src/app/impl/layout LOCAL_SRC_LIST)
aux_source_directory(src/app/impl/narrow_phrase LOCAL_SRC_LIST)
aux_source_directory(src/app/impl/renderer LOCAL_SRC_LIST)
aux_source_directory(src/app/inf LOCAL_SRC_LIST)
aux_source_directory(src/app/util LOCAL_SRC_LIST)
aux_source_directory(src/app/view LOCAL_SRC_LIST)

aux_source_directory(platform/any LOCAL_SRC_LIST)
ark_include_directories(platform/any)

ark_find_vcpkg_package(tinyxml2 LIBRARIES tinyxml2::tinyxml2)
ark_find_vcpkg_package(yaml LIBRARIES yaml)

ark_find_vcpkg_package(libzip LIBRARIES libzip::zip)

ark_find_vcpkg_package(glm LIBRARIES glm::glm)

ark_include_directories(3rdparty/cute_headers)

ark_find_vcpkg_package(nlohmann_json LIBRARIES nlohmann_json::nlohmann_json)
ark_find_vcpkg_package(spdlog LIBRARIES spdlog::spdlog spdlog::spdlog_header_only)

if(ARK_USE_LIBPNG)
    ark_find_vcpkg_package(PNG LIBRARIES PNG::PNG MODULE_MODE)
endif()

if(ARK_USE_LIBJPEG_TURBO)
    ark_find_vcpkg_package(JPEG MODULE_MODE)
    ark_link_libraries(${JPEG_LIBRARIES})
    ark_include_directories(${JPEG_INCLUDE_DIR})
endif()

if(ARK_USE_STB_IMAGE)
    find_path(LOCAL_STB_INCLUDE_DIRS "stb_c_lexer.h")
    ark_include_directories(${LOCAL_STB_INCLUDE_DIRS})
endif()

ark_find_vcpkg_package(freetype LIBRARIES freetype)

ark_add_plugin_directory(python plugin/python)
ark_add_plugin_directory(box2d plugin/box2d)
ark_add_plugin_directory(vorbis plugin/vorbis)
ark_add_plugin_directory(miniaudio plugin/miniaudio)
ark_add_plugin_directory(noise plugin/noise)
ark_add_plugin_directory(fmod plugin/fmod)
ark_add_plugin_directory(assimp plugin/assimp)
ark_add_plugin_directory(cmft plugin/cmft)
ark_add_plugin_directory(dear_imgui plugin/dear-imgui)
ark_add_plugin_directory(bullet plugin/bullet)
ark_add_plugin_directory(optick plugin/optick)
ark_add_plugin_directory(yoga plugin/yoga)

if(NOT (ANDROID OR IOS))
    ark_find_vcpkg_package(SDL2 LIBRARIES $<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>)
endif()

ark_setup_tools(ark_gen_string_table)
ark_setup_tools(ark_gen_plugin_class)
ark_setup_tools(ark_gen_bootstrap)
ark_setup_tools(ark_gen_class_hierarchy)

ark_gen_builtin_plugin_class(ark ark framework_plugin src/core src/graphics src/renderer/base src/renderer/impl src/renderer/util)

if(ARK_USE_OPEN_GL)
    ark_gen_plugin_class(opengl builtin ark::opengl opengl_plugin src/renderer/opengl)
endif()

if(ARK_USE_VULKAN)
    include(src/renderer/vulkan/include.cmake)
endif()

ark_gen_plugin_class(app builtin ark application_plugin src/app)

ark_gen_class_hierarchy(ark ark ${ARK_SRC_DIR}/src core graphics renderer/base renderer/impl renderer/inf app)

ark_gen_string_table(shaders ark::res src/renderer/shaders)
ark_gen_bootstrap(ark_bootstrap)

ark_include_directories(${CMAKE_CURRENT_BINARY_DIR})

if(ARK_BUILD_STATIC_PLUGINS)
    list(APPEND LOCAL_SHARED_LIBRARY_DEPENDENCIES ${ARK_DEPENDENCY_LIBRARIES})
    ark_add_generated_file(tools/python/gen_static_plugin_libraries.py ${ARK_GENERATED_FILE_DIRECTORY}/gen_static_libraries.cpp ${CMAKE_SOURCE_DIR} "${ARK_STATIC_PLUGIN_DEPENDCIES}" ${ARK_STATIC_PLUGINS})
endif()
list(APPEND LOCAL_SHARED_LIBRARY_DEPENDENCIES ${LOCAL_SRC_LIST} ${LOCAL_GENERATED_SRC_LIST} ${LOCAL_RUNTIME_LIBRARY_SRC_LIST} ${LOCAL_RES_LIST})

if(ARK_BUILD_TESTS)
    set(GENERATED_RUN_TESTS ${ARK_GENERATED_FILE_DIRECTORY}/testcase_launchpad.cpp)
    aux_source_directory(test/base TEST_SRC_LIST)
    aux_source_directory(test/testcase TEST_SRC_LIST)
    add_custom_command(OUTPUT ${GENERATED_RUN_TESTS}
        COMMAND ${Python_EXECUTABLE} ${ARK_SRC_DIR}/tools/python/gen_testcase_launchpad.py ${ARK_SRC_DIR}/test/testcase ${GENERATED_RUN_TESTS}
        DEPENDS ${TEST_SRC_LIST} ${ARK_SRC_DIR}/tools/python/gen_testcase_launchpad.py
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
    list(APPEND TEST_SRC_LIST ${GENERATED_RUN_TESTS})
endif()

ark_add_shared_library(${ARK_RUNTIME_SHARED_LIBRARY} ${LOCAL_SHARED_LIBRARY_DEPENDENCIES} ${TEST_SRC_LIST})
target_compile_definitions(${ARK_RUNTIME_SHARED_LIBRARY} PRIVATE -DARK_FLAG_BUILDING_FRAMEWORK)
target_link_libraries(${ARK_RUNTIME_SHARED_LIBRARY} PRIVATE ${ARK_DEPENDENCY_3RDPARTY_LIBRARIES})

if(ARK_BUILD_TESTS AND NOT ARK_BUILD_STATIC_LIBRARY)
    ark_add_executable(${TESTCASES_EXECUTABLE} test/main.cpp ${ARK_RUNTIME_SHARED_LIBRARY})
endif()

if(ARK_BUILD_STATIC_LIBRARY)
    ark_add_static_library(${ARK_RUNTIME_STATIC_LIBRARY} ${LOCAL_SRC_LIST} ${LOCAL_GENERATED_SRC_LIST} ${TEST_SRC_LIST})
    target_compile_definitions(${ARK_RUNTIME_STATIC_LIBRARY} PRIVATE -DARK_FLAG_BUILDING_FRAMEWORK)
    target_compile_definitions(${ARK_RUNTIME_STATIC_LIBRARY} PUBLIC -DARK_BUILD_STATIC)
    if(ARK_BUILD_TESTS)
        ark_add_executable(${TESTCASES_EXECUTABLE} test/main.cpp ${ARK_RUNTIME_STATIC_LIBRARY} ${ARK_DEPENDENCY_LIBRARIES})
    endif()
endif()

ark_export_libraries(${ARK_LIBRARIES})

if(NOT ANDROID AND NOT IOS)
    ark_add_executable(ark-launcher src/app/main/main.cpp)
    target_link_libraries(ark-launcher PRIVATE ${ARK_LIBRARIES})
endif()

configure_file(ark-config.cmake.in "${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/${PROJECT_NAME}-config.cmake" @ONLY)

if(ARK_BUILD_STATIC_LIBRARY)
    ark_export_libraries(${ARK_STATIC_LIBRARIES} ${ARK_DEPENDENCY_LIBRARIES})
    list(REMOVE_DUPLICATES ARK_EXPORT_LIBRARIES)
endif()

export(TARGETS ${ARK_EXPORT_LIBRARIES} FILE "${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/${PROJECT_NAME}-targets.cmake")

if(WIN32 AND NOT CYGWIN)
  set(DEF_INSTALL_CMAKE_DIR CMake)
else()
  set(DEF_INSTALL_CMAKE_DIR lib/CMake/${PROJECT_NAME})
endif()
set(INSTALL_CMAKE_DIR ${DEF_INSTALL_CMAKE_DIR} CACHE PATH "Installation directory for CMake files")

install(TARGETS ${ARK_RUNTIME_SHARED_LIBRARY}
        EXPORT  ark-targets
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib/static)
install(EXPORT ark-targets DESTINATION ${INSTALL_CMAKE_DIR})
