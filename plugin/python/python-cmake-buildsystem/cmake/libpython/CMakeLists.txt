add_definitions(-DPy_BUILD_CORE)
add_definitions(-DNDEBUG)

if(BUILD_EXTENSIONS_AS_BUILTIN)
    add_definitions(-DPy_BUILD_CORE_BUILTIN)
else()
    add_definitions(-DPy_BUILD_CORE_MODULE)
endif()

set(LIBPYTHON_SOURCES
    ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/config.c
#    ${SRC_DIR}/Modules/_codecsmodule.c
    ${SRC_DIR}/Modules/errnomodule.c
    ${SRC_DIR}/Modules/faulthandler.c
    ${SRC_DIR}/Modules/gcmodule.c
    ${SRC_DIR}/Modules/getbuildinfo.c
    ${SRC_DIR}/Modules/main.c
    ${SRC_DIR}/Modules/signalmodule.c
    ${SRC_DIR}/Modules/_sre.c
    ${SRC_DIR}/Modules/symtablemodule.c
    ${SRC_DIR}/Modules/_tracemalloc.c
#    ${SRC_DIR}/Modules/_weakref.c
    ${SRC_DIR}/Modules/xxsubtype.c
#    ${SRC_DIR}/Modules/zipimport.c
#    ${SRC_DIR}/Modules/hashtable.c
    ${SRC_DIR}/Modules/_bisectmodule.c
    ${SRC_DIR}/Objects/abstract.c
    ${SRC_DIR}/Objects/accu.c
    ${SRC_DIR}/Objects/boolobject.c
#    ${SRC_DIR}/Objects/bufferobject.c
    ${SRC_DIR}/Objects/bytesobject.c
    ${SRC_DIR}/Objects/bytearrayobject.c
    ${SRC_DIR}/Objects/bytes_methods.c
    ${SRC_DIR}/Objects/call.c
    ${SRC_DIR}/Objects/capsule.c
    ${SRC_DIR}/Objects/cellobject.c
    ${SRC_DIR}/Objects/classobject.c
#    ${SRC_DIR}/Objects/cobject.c
    ${SRC_DIR}/Objects/codeobject.c
    ${SRC_DIR}/Objects/complexobject.c
    ${SRC_DIR}/Objects/descrobject.c
    ${SRC_DIR}/Objects/dictobject.c
    ${SRC_DIR}/Objects/enumobject.c
    ${SRC_DIR}/Objects/exceptions.c
    ${SRC_DIR}/Objects/fileobject.c
    ${SRC_DIR}/Objects/floatobject.c
    ${SRC_DIR}/Objects/frameobject.c
    ${SRC_DIR}/Objects/funcobject.c
    ${SRC_DIR}/Objects/genericaliasobject.c
    ${SRC_DIR}/Objects/genobject.c
#    ${SRC_DIR}/Objects/intobject.c
    ${SRC_DIR}/Objects/interpreteridobject.c
    ${SRC_DIR}/Objects/iterobject.c
    ${SRC_DIR}/Objects/listobject.c
    ${SRC_DIR}/Objects/longobject.c
    ${SRC_DIR}/Objects/memoryobject.c
    ${SRC_DIR}/Objects/methodobject.c
    ${SRC_DIR}/Objects/moduleobject.c
    ${SRC_DIR}/Objects/namespaceobject.c
    ${SRC_DIR}/Objects/object.c
    ${SRC_DIR}/Objects/obmalloc.c
    ${SRC_DIR}/Objects/odictobject.c
    ${SRC_DIR}/Objects/picklebufobject.c
    ${SRC_DIR}/Objects/rangeobject.c
    ${SRC_DIR}/Objects/setobject.c
    ${SRC_DIR}/Objects/sliceobject.c
#    ${SRC_DIR}/Objects/stringobject.c
    ${SRC_DIR}/Objects/structseq.c
    ${SRC_DIR}/Objects/tupleobject.c
    ${SRC_DIR}/Objects/typeobject.c
    ${SRC_DIR}/Objects/unicodectype.c
    ${SRC_DIR}/Objects/unicodeobject.c
    ${SRC_DIR}/Objects/unionobject.c
    ${SRC_DIR}/Objects/weakrefobject.c
#    ${SRC_DIR}/Parser/acceler.c
#    ${SRC_DIR}/Parser/bitset.c
#    ${SRC_DIR}/Parser/firstsets.c
#    ${SRC_DIR}/Parser/grammar1.c
#    ${SRC_DIR}/Parser/grammar.c
#    ${SRC_DIR}/Parser/listnode.c
#    ${SRC_DIR}/Parser/metagrammar.c
    ${SRC_DIR}/Parser/myreadline.c
#    ${SRC_DIR}/Parser/node.c
    ${SRC_DIR}/Parser/parser.c
#    ${SRC_DIR}/Parser/parsetok.c
    ${SRC_DIR}/Parser/peg_api.c
    ${SRC_DIR}/Parser/pegen.c
    ${SRC_DIR}/Parser/string_parser.c
    ${SRC_DIR}/Parser/token.c
    ${SRC_DIR}/Parser/tokenizer.c
    ${SRC_DIR}/Python/asdl.c
    ${SRC_DIR}/Python/ast.c
    ${SRC_DIR}/Python/ast_opt.c
    ${SRC_DIR}/Python/ast_unparse.c
    ${SRC_DIR}/Python/bltinmodule.c
    ${SRC_DIR}/Python/bootstrap_hash.c
    ${SRC_DIR}/Python/ceval.c
    ${SRC_DIR}/Python/codecs.c
    ${SRC_DIR}/Python/compile.c
    ${SRC_DIR}/Python/context.c
    ${SRC_DIR}/Python/dtoa.c
    ${SRC_DIR}/Python/errors.c
#    ${SRC_DIR}/Python/formatter_string.c
    ${SRC_DIR}/Python/fileutils.c
    ${SRC_DIR}/Python/formatter_unicode.c
    ${SRC_DIR}/Python/frozen.c
    ${SRC_DIR}/Python/future.c
    ${SRC_DIR}/Python/getargs.c
    ${SRC_DIR}/Python/getcompiler.c
    ${SRC_DIR}/Python/getcopyright.c
    ${SRC_DIR}/Python/getopt.c
    ${SRC_DIR}/Python/getplatform.c
    ${SRC_DIR}/Python/getversion.c
#    ${SRC_DIR}/Python/graminit.c
    ${SRC_DIR}/Python/hamt.c
    ${SRC_DIR}/Python/hashtable.c
    ${SRC_DIR}/Python/import.c
    ${SRC_DIR}/Python/importdl.c
    ${SRC_DIR}/Python/initconfig.c
    ${SRC_DIR}/Python/marshal.c
    ${SRC_DIR}/Python/modsupport.c
    ${SRC_DIR}/Python/mysnprintf.c
    ${SRC_DIR}/Python/mystrtoul.c
    ${SRC_DIR}/Python/pathconfig.c
    ${SRC_DIR}/Python/preconfig.c
#    ${SRC_DIR}/Python/peephole.c
    ${SRC_DIR}/Python/pyarena.c
    ${SRC_DIR}/Python/pyctype.c
    ${SRC_DIR}/Python/pyfpe.c
    ${SRC_DIR}/Python/pyhash.c
    ${SRC_DIR}/Python/pylifecycle.c
    ${SRC_DIR}/Python/pymath.c
    ${SRC_DIR}/Python/pystate.c
    ${SRC_DIR}/Python/pystrcmp.c
    ${SRC_DIR}/Python/pystrhex.c
    ${SRC_DIR}/Python/pystrtod.c
    ${SRC_DIR}/Python/pytime.c
    ${SRC_DIR}/Python/Python-ast.c
    ${SRC_DIR}/Python/pythonrun.c
#    ${SRC_DIR}/Python/random.c
    ${SRC_DIR}/Python/structmember.c
    ${SRC_DIR}/Python/suggestions.c
    ${SRC_DIR}/Python/symtable.c
    ${SRC_DIR}/Python/sysmodule.c
    ${SRC_DIR}/Python/traceback.c
    ${SRC_DIR}/Python/thread.c
    ${SRC_DIR}/Python/_warnings.c
)

if(CMAKE_C_COMPILER_ID MATCHES GNU)
    foreach(filename class complex float int method string type unicode weakref)
        set_property(SOURCE ${SRC_DIR}/Objects/${filename}object.c PROPERTY COMPILE_FLAGS -Wno-unused-value)
    endforeach()
endif()

if(WITH_THREAD)
    list(APPEND LIBPYTHON_SOURCES
        ${SRC_DIR}/Python/thread.c
    )
endif()

if(UNIX OR MINGW)
    set_property(
        SOURCE ${SRC_DIR}/Python/getplatform.c
        PROPERTY COMPILE_DEFINITIONS
            PLATFORM="${PY_PLATFORM}"
    )
endif(UNIX OR MINGW)

# Platform-specific source files
if(UNIX)
    list(APPEND LIBPYTHON_SOURCES
        ${SRC_DIR}/Python/dynload_shlib.c
        ${SRC_DIR}/Modules/getpath.c
    )
    if(CMAKE_C_COPILER_ID MATCHES GNU)
        set_property(
            SOURCE ${SRC_DIR}/Modules/getpath.c
            PROPERTY COMPILE_FLAGS
                -Wno-deprecated-declarations
            )
    endif()
    set(PYTHONPATH "${EXTRA_PYTHONPATH}:lib-dynload:plat-${PY_PLATFORM}")
    if(ENABLE_TKINTER)
        set(PYTHONPATH "${PYTHONPATH}:lib-tk")
    endif(ENABLE_TKINTER)
    set_property(
        SOURCE ${SRC_DIR}/Modules/getpath.c
        PROPERTY COMPILE_DEFINITIONS
            PREFIX="${CMAKE_INSTALL_PREFIX}"
            EXEC_PREFIX="${CMAKE_INSTALL_PREFIX}"
            VERSION="${PY_VERSION_MAJOR}.${PY_VERSION_MINOR}"
            VPATH="."
            PYTHONPATH="${PYTHONPATH}"
      )
elseif(WIN32)
    # HACK To workaround limitation in escaping logic of CMake, the pythonpath
    #      separator is conditionally set depending of the version of Visual Studio.
    #      See http://cmake.org/Bug/view.php?id=14073
    if( ("${MSVC_VERSION}" VERSION_GREATER "1599") AND ("${CMAKE_GENERATOR}" MATCHES "^Visual Studio") )
        set(PATHSEP "%3B")
    elseif( (${MSVC}) AND ("${CMAKE_GENERATOR}" MATCHES "^Ninja") )
        set(PATHSEP "\;")
    elseif( (${MSVC}) AND ("${CMAKE_GENERATOR}" MATCHES "^NMake") )
        set(PATHSEP "\;")
    else()
        set(PATHSEP ";")
    endif()
    list(APPEND LIBPYTHON_SOURCES
        ${SRC_DIR}/PC/dl_nt.c
        ${SRC_DIR}/PC/getpathp.c
#        ${SRC_DIR}/PC/import_nt.c
        ${SRC_DIR}/PC/invalid_parameter_handler.c
        ${SRC_DIR}/Python/dynload_win.c
    )
    string(REPLACE "/" "\\\\" PYTHONHOME_ESCAPED ${PYTHONHOME})
    string(REPLACE "/" "\\\\" EXTENSION_INSTALL_DIR_ESCAPED ${EXTENSION_INSTALL_DIR})
    set(PYTHONPATH "${EXTRA_PYTHONPATH}")
    set(PYTHONPATH "${PYTHONPATH}${PATHSEP}.\\\\${PYTHONHOME_ESCAPED}")
    set(PYTHONPATH "${PYTHONPATH}${PATHSEP}.\\\\${EXTENSION_INSTALL_DIR_ESCAPED}")
    set(PYTHONPATH "${PYTHONPATH}${PATHSEP}.\\\\${EXTENSION_INSTALL_DIR_ESCAPED}\\\\${CMAKE_CFG_INTDIR}")
    set(PYTHONPATH "${PYTHONPATH}${PATHSEP}.\\\\${PYTHONHOME_ESCAPED}\\\\plat-${PY_PLATFORM}")
    if(ENABLE_TKINTER)
        set(PYTHONPATH "${PYTHONPATH}${PATHSEP}.\\\\${PYTHONHOME_ESCAPED}\\\\lib-tk")
    endif(ENABLE_TKINTER)
    
    set_property(
        SOURCE ${SRC_DIR}/PC/getpathp.c
        PROPERTY COMPILE_DEFINITIONS
            LANDMARK="${PYTHONHOME_ESCAPED}\\\\os.py"
            "PYTHONPATH=\"${PYTHONPATH}\""
    )
    set_property(
        SOURCE ${SRC_DIR}/PC/dl_nt.c
        PROPERTY COMPILE_DEFINITIONS Py_ENABLE_SHARED
    )
    add_definitions(-DPY3_DLLNAME="python310.dll" -DMS_DLL_ID="3.10")
endif(UNIX)

# List of builtin extensions
get_property(builtin_extensions GLOBAL PROPERTY builtin_extensions)

# Collect builtin extension sources
#set(builtin_extension_sources)
#foreach(name ${builtin_extensions})
#    get_property(extension_${name}_sources GLOBAL PROPERTY extension_${name}_sources)
#    list(APPEND builtin_extension_sources ${extension_${name}_sources})
#endforeach()

# XXX Associate Py_NO_ENABLE_SHARED with "_ctypes/callbacks.c" if corresponding
#     extension is builtin.
#if(WIN32)
#    foreach(src ${builtin_extension_sources})
#        get_filename_component(filename ${src} NAME)
#        if("${filename}" STREQUAL "callbacks.c")
#            set_property(SOURCE ${src} APPEND PROPERTY COMPILE_DEFINITIONS Py_NO_ENABLE_SHARED)
#        endif()
#    endforeach()
#endif()

# Collect builtin extension link libraries
set(builtin_link_libraries)
foreach(name ${builtin_extensions})
    get_property(extension_${name}_link_libraries GLOBAL PROPERTY extension_${name}_link_libraries)
    list(APPEND builtin_link_libraries ${extension_${name}_link_libraries})
endforeach()

# Collect builtin extension includedirs
#set(builtin_includedirs)
#foreach(name ${builtin_extensions})
#    get_property(extension_${name}_includedirs GLOBAL PROPERTY extension_${name}_includedirs)
#    list(APPEND builtin_includedirs ${extension_${name}_includedirs})
#endforeach()

## Collect builtin extension definitions
#foreach(name ${builtin_extensions})
#    get_property(extension_${name}_definitions GLOBAL PROPERTY extension_${name}_definitions)
#    if(extension_${name}_definitions)
#        set_property(SOURCE ${extension_${name}_sources}
#            APPEND PROPERTY COMPILE_DEFINITIONS ${extension_${name}_definitions})
#    endif()
#endforeach()

# Create the parts of config.c for platform-specific and user-controlled
# builtin modules.

include_directories(${builtin_includedirs})

set(config_inits "")
set(config_entries "")
foreach(ext ${builtin_extensions})
    set(config_inits "${config_inits}extern PyObject* PyInit_${ext}(void);\n")
    set(config_entries "${config_entries}    {\"${ext}\", PyInit_${ext}},\n")
endforeach(ext)

configure_file(
    ${PROJECT_SOURCE_DIR}/cmake/config.c.in
    ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/config.c
    )


function(add_libpython name type)
    add_library(${name} ${type} ${LIBPYTHON_SOURCES} ${builtin_extension_sources})
    target_link_libraries(${name}
        ${builtin_link_libraries}
        ${HAVE_LIBDL}
    )
    if(WITH_THREAD)
        target_link_libraries(${name} ${CMAKE_THREAD_LIBS_INIT})
    endif(WITH_THREAD)

#    if(UNIX)
#        target_link_libraries(${name} ${HAVE_LIBUTIL} ${HAVE_LIBM})
#    endif(UNIX)

    set_target_properties(${name} PROPERTIES
        OUTPUT_NAME ${LIBPYTHON}
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${LIBPYTHON_LIBDIR}
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${LIBPYTHON_LIBDIR}
        INSTALL_NAME_DIR ${CMAKE_INSTALL_PREFIX}/${LIBPYTHON_LIBDIR}
        POSITION_INDEPENDENT_CODE ON
    )
    if(HAVE_POSITION_INDEPENDENT_CODE)
        set_target_properties(${name} PROPERTIES
            POSITION_INDEPENDENT_CODE ON
        )
    endif()

    # Export target
    set_property(GLOBAL APPEND PROPERTY PYTHON_TARGETS ${name})

    install(TARGETS ${name} EXPORT PythonTargets
        ARCHIVE DESTINATION ${LIBPYTHON_ARCHIVEDIR}
        LIBRARY DESTINATION ${LIBPYTHON_LIBDIR}
        RUNTIME DESTINATION ${LIBPYTHON_LIBDIR}
    )
endfunction(add_libpython)

if(BUILD_SHARED)
    add_libpython(libpython-shared SHARED)
    set_target_properties(libpython-shared PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${LIBPYTHON_ARCHIVEDIR}
    )
    if(APPLE)
        # HACK For python <= 2.7.3, this fix link error related to undefined _environ symbol and
        #      is equivalent to solution implemented in commit http://hg.python.org/cpython/rev/864b983
        #      The property is set here because source file properties can't be in one directory
        #      and used to build a target in an other directory.
#        set_property(
#            SOURCE ${SRC_DIR}/Modules/posixmodule.c
#            APPEND PROPERTY COMPILE_DEFINITIONS WITH_NEXT_FRAMEWORK)
    endif(APPLE)
    if(WIN32)
        target_link_libraries(libpython-shared version.lib Shlwapi.lib Pathcch.lib Iphlpapi.lib)
    endif()
    if(ARK_PYTHON_PATCH_LIBS)
        target_link_libraries(libpython-shared ${ARK_PYTHON_PATCH_LIBS})
    endif()
    if(LOCAL_PYTHON_PATCH_COMPILE_OPTIONS)
        target_compile_options(libpython-shared PRIVATE ${LOCAL_PYTHON_PATCH_COMPILE_OPTIONS})
    endif()
endif(BUILD_SHARED)

if(BUILD_STATIC)
    add_libpython(libpython-static STATIC)
    if(HAVE_TARGET_COMPILE_DEFINITIONS)
        target_compile_definitions(libpython-static PUBLIC Py_NO_ENABLE_SHARED)
    else(HAVE_TARGET_COMPILE_DEFINITIONS)
        set_target_properties(libpython-static PROPERTIES
            COMPILE_DEFINITIONS Py_NO_ENABLE_SHARED
        )
    endif(HAVE_TARGET_COMPILE_DEFINITIONS)
    set_target_properties(libpython-static PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${LIBPYTHON_STATIC_ARCHIVEDIR}
    )
    install(TARGETS libpython-static ARCHIVE DESTINATION ${PYTHONHOME}/config/)
#    if(WIN32)
#        target_link_libraries(libpython-static version.lib Shlwapi.lib Pathcch.lib Iphlpapi.lib)
#    endif()
    if(ARK_PYTHON_PATCH_LIBS)
        target_link_libraries(libpython-static ${ARK_PYTHON_PATCH_LIBS})
    endif()
    if(LOCAL_PYTHON_PATCH_COMPILE_OPTIONS)
        target_compile_options(libpython-static PRIVATE ${LOCAL_PYTHON_PATCH_COMPILE_OPTIONS})
    endif()
endif(BUILD_STATIC)
