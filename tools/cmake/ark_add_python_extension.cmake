# This function adds a python extension to the buildsystem.
#
# Usage:
#
# ark_add_python_extension(
#     extension_name
#     SOURCES source1.c source2.c ...
#     [ REQUIRES variable1 variable2 ... ]
#     [ DEFINITIONS define1 define2 ... ]
#     [ LIBRARIES lib1 lib2 ... ]
#     [ INCLUDEDIRS dir1 dir2 ... ]
#     [ BUILTIN ]
# )
#
# extension_name: the name of the library without any .so extension.
# SOURCES:     a list of filenames realtive to the Modules/ directory that make
#              up this extension.
# REQUIRES:    this extension will not be built unless all the variables listed
#              here evaluate to true.  You should include any variables you use
#              in the LIBRARIES and INCLUDEDIRS sections.
# DEFINITIONS: an optional list of definitions to pass to the compiler while
#              building this module.  Do not include the -D prefix.
# LIBRARIES:   an optional list of additional libraries.
# INCLUDEDIRS: an optional list of additional include directories.
# BUILTIN:     if this is set the module will be compiled statically into
#              libpython by default.  The user can still override by setting
#              BUILTIN_[extension_name]=OFF.
#
# Two user-settable options are created for each extension added:
# ENABLE_[extension_name]   defaults to ON.  If set to OFF the extension will
#                           not be added at all.
# BUILTIN_[extension_name]  defaults to the value of
#                           BUILD_EXTENSIONS_AS_BUILTIN, which defaults to OFF,
#                           unless BUILTIN is set when calling
#                           ark_add_python_extension.  Adds the extension source
#                           files to libpython instead of compiling a separate
#                           library.
# These options convert the extension_name to upper case first and remove any
# leading underscores.  So ark_add_python_extension(_foo ...) will create the
# options ENABLE_FOO and BUILTIN_FOO.

function(ark_add_python_extension name)
    set(options BUILTIN)
    set(oneValueArgs)
    set(multiValueArgs REQUIRES SOURCES WIN32_SOURCES DEFINITIONS LIBRARIES INCLUDEDIRS)
    cmake_parse_arguments(ARK_ADD_PYTHON_EXTENSION
        "${options}"
        "${oneValueArgs}"
        "${multiValueArgs}"
        ${ARGN}
        )

    # Remove _ from the beginning of the name.
    string(REGEX REPLACE "^_" "" pretty_name "${name}")

    # Upper case the name.
    string(TOUPPER "${pretty_name}" upper_name)

    # Add a prefix to the target name so it doesn't clash with any system
    # libraries that we might want to link against (eg. readline)
    set(target_name extension_${pretty_name})

    # Add options that the user can set to control whether this extension is
    # compiled, and whether it is compiled in to libpython itself.
    option(ARK_PYTHON_EXTENSION_ENABLE_${upper_name}
           "Controls whether the \"${name}\" extension will be built"
           ON
    )
    if(ARK_PYTHON_EXTENSION_ENABLE_${upper_name})
        mark_as_advanced(FORCE ARK_PYTHON_EXTENSION_ENABLE_${upper_name})
    else()
        mark_as_advanced(CLEAR ARK_PYTHON_EXTENSION_ENABLE_${upper_name})
    endif()

    # Check all the things we require are found.
    set(missing_deps "")
    foreach(dep ${ARK_ADD_PYTHON_EXTENSION_REQUIRES} ARK_PYTHON_EXTENSION_ENABLE_${upper_name})
        string(REPLACE " " ";" list_dep ${dep})
        if(NOT (${list_dep}))
            set(missing_deps "${missing_deps}${dep} ")
        endif()
    endforeach()

    # Add options that the extention is either external to libpython or
    # builtin.  These will be marked as advanced unless different from default
    # values
    if(NOT ARK_ADD_PYTHON_EXTENSION_BUILTIN)
        set(ARK_ADD_PYTHON_EXTENSION_BUILTIN ${ARK_PYTHON_EXTENSION_BUILD_AS_BUILTIN})
    endif()
    cmake_dependent_option(
        ARK_PYTHON_EXTENSION_BUILTIN_${upper_name}
        "If this is set the \"${name}\" extension will be compiled in to libpython"
        ${ARK_ADD_PYTHON_EXTENSION_BUILTIN}
        "NOT missing_deps"
        OFF
    )
    if(NOT missing_deps)
        if((ARK_PYTHON_EXTENSION_BUILTIN_${upper_name} AND ARK_PYTHON_EXTENSION_BUILD_AS_BUILTIN)
            OR (NOT ARK_PYTHON_EXTENSION_BUILTIN_${upper_name} AND NOT ARK_PYTHON_EXTENSION_BUILD_AS_BUILTIN))
            mark_as_advanced(FORCE ARK_PYTHON_EXTENSION_BUILTIN_${upper_name})
        else()
            mark_as_advanced(CLEAR ARK_PYTHON_EXTENSION_BUILTIN_${upper_name})
        endif()
    endif()

    # XXX _ctypes_test and _testcapi should always be shared
    if(${name} STREQUAL "_ctypes_test" OR ${name} STREQUAL "_testcapi")
        unset(ARK_PYTHON_EXTENSION_BUILTIN_${upper_name} CACHE)
        set(ARK_PYTHON_EXTENSION_BUILTIN_${upper_name} 0)
    endif()

    # If any dependencies were missing don't include this extension.
    if(missing_deps)
        string(STRIP "${missing_deps}" missing_deps)
        set(extensions_disabled "${extensions_disabled}${name} (not set: ${missing_deps});"
             CACHE INTERNAL "" FORCE)
        return()
    else(missing_deps)
        set(extensions_enabled "${extensions_enabled}${name};" CACHE INTERNAL "" FORCE)
    endif(missing_deps)

    # Callers to this function provide source files relative to the Modules/
    # directory.  We need to get absolute paths for them all.

    if(WIN32)
        list(APPEND ARK_ADD_PYTHON_EXTENSION_SOURCES ${ARK_ADD_PYTHON_EXTENSION_WIN32_SOURCES})
    endif()

    set(absolute_sources "")
    foreach(source ${ARK_ADD_PYTHON_EXTENSION_SOURCES})
        get_filename_component(ext ${source} EXT)

        # Treat assembler sources differently
        if(ext STREQUAL ".S")
            set_source_files_properties(Modules/${source} PROPERTIES LANGUAGE ASM)
        endif()
        set(absolute_src ${source})
        if(NOT IS_ABSOLUTE ${source})
            set(absolute_src ${ARK_PYTHON_SRC_DIR}/Modules/${source})
        endif(NOT IS_ABSOLUTE ${source})
        list(APPEND absolute_sources ${absolute_src})
        if(WIN32)
            get_filename_component(filename ${absolute_src} NAME)
            if("${filename}" STREQUAL "callbacks.c")
                set_property(SOURCE ${absolute_src} APPEND PROPERTY COMPILE_DEFINITIONS Py_NO_ENABLE_SHARED)
            endif()
        endif()
    endforeach(source)

    if(ARK_PYTHON_EXTENSION_BUILTIN_${upper_name})
        # This will be compiled into libpython instead of as a separate library
        add_library(${target_name} STATIC ${absolute_sources})
        set_property(GLOBAL APPEND PROPERTY builtin_extensions ${name})
        set_property(GLOBAL APPEND PROPERTY extension_${name}_link_libraries ${target_name} ${ARK_ADD_PYTHON_EXTENSION_LIBRARIES})
        target_include_directories(${target_name} PRIVATE ${ARK_ADD_PYTHON_EXTENSION_INCLUDEDIRS})
        target_compile_definitions(${target_name} PRIVATE -DPy_BUILD_CORE -DPy_BUILD_CORE_BUILTIN ${ARK_ADD_PYTHON_EXTENSION_DEFINITIONS})
    elseif(WIN32 AND NOT BUILD_SHARED)
        # Extensions cannot be built against a static libpython on windows
    else(ARK_PYTHON_EXTENSION_BUILTIN_${upper_name})
        add_library(${target_name} SHARED ${absolute_sources})
        target_include_directories(${target_name} PRIVATE ${ARK_ADD_PYTHON_EXTENSION_INCLUDEDIRS})
        target_link_libraries(${target_name} PRIVATE ${ARK_ADD_PYTHON_EXTENSION_LIBRARIES})

        if(WIN32)
            #list(APPEND ARK_ADD_PYTHON_EXTENSION_DEFINITIONS Py_NO_ENABLE_SHARED)
            target_link_libraries(${target_name} libpython-shared)
            if(MINGW)
                set_target_properties(${target_name} PROPERTIES
                    LINK_FLAGS -Wl,--enable-auto-import
                )
            endif(MINGW)
            set_target_properties(${target_name} PROPERTIES
                SUFFIX .pyd
            )
        endif(WIN32)
        
        if(APPLE)
            set_target_properties(${target_name} PROPERTIES
                LINK_FLAGS -Wl,-undefined,dynamic_lookup
                SUFFIX .so
            )
        endif(APPLE)

        # Turn off the "lib" prefix and add any compiler definitions
        set_target_properties(${target_name} PROPERTIES
            ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${ARCHIVEDIR}
            LIBRARY_OUTPUT_DIRECTORY ${EXTENSION_BUILD_DIR}
            RUNTIME_OUTPUT_DIRECTORY ${EXTENSION_BUILD_DIR}
            OUTPUT_NAME "${name}"
            PREFIX ""
        )
        if(HAVE_POSITION_INDEPENDENT_CODE)
            set_target_properties(${target_name} PROPERTIES
                POSITION_INDEPENDENT_CODE ON
            )
        endif()

        if(ARK_ADD_PYTHON_EXTENSION_DEFINITIONS)
            target_compile_definitions(${target_name} PUBLIC ${ARK_ADD_PYTHON_EXTENSION_DEFINITIONS})
        endif()

        install(TARGETS ${target_name}
                ARCHIVE DESTINATION ${ARCHIVEDIR}
                LIBRARY DESTINATION ${EXTENSION_INSTALL_DIR}
                RUNTIME DESTINATION ${EXTENSION_INSTALL_DIR})
    endif()
endfunction()


function(ark_show_extension_summary)
    if(ark_extensions_disabled)
        message(STATUS "")
        message(STATUS "The following extensions will NOT be built:")
        message(STATUS "")
        foreach(line ${extensions_disabled})
            message(STATUS "    ${line}")
        endforeach(line)
        message(STATUS "")
    endif()
endfunction()
