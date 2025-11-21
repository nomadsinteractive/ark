ark_setup_tools(ark_add_python_extension)
set(LOCAL_PYTHON_INCLUDE_DIRS ${ARK_PYTHON_INCLUDE_DIR} ${ARK_PYTHON_SRC_DIR}/Include/internal ${ARK_PYTHON_SRC_DIR}/Modules ${LOCAL_INCLUDE_DIRS})

ark_find_vcpkg_package(unofficial-libffi LIBRARIES unofficial::libffi::libffi)
ark_add_python_extension(_ctypes
          SOURCES _ctypes/_ctypes.c
                  _ctypes/callbacks.c
                  _ctypes/callproc.c
                  _ctypes/cfield.c
                  _ctypes/malloc_closure.c
                  _ctypes/stgdict.c
          BUILTIN
          INCLUDEDIRS
              ${LOCAL_PYTHON_INCLUDE_DIRS} ${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/include
          DEFINITIONS Py_ENABLE_SHARED Py_BUILD_CORE_MODULE FFI_BUILDING
          LIBRARIES unofficial::libffi::libffi ${ARK_PYTHON_LIBRARIES}
        )

list(APPEND _decimal_SOURCES
    _decimal/_decimal.c
    _decimal/libmpdec/basearith.c
    _decimal/libmpdec/constants.c
    _decimal/libmpdec/convolute.c
    _decimal/libmpdec/context.c
    _decimal/libmpdec/numbertheory.c
    _decimal/libmpdec/fnt.c
    _decimal/libmpdec/difradix2.c
    _decimal/libmpdec/transpose.c
    _decimal/libmpdec/sixstep.c
    _decimal/libmpdec/fourstep.c
    _decimal/libmpdec/crt.c
    _decimal/libmpdec/io.c
    _decimal/libmpdec/mpalloc.c
    _decimal/libmpdec/mpdecimal.c
)
if(MSVC)
    set(_decimal_DEFINITIONS MASM)
else()
    set(_decimal_DEFINITIONS ASM)
endif()

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    list(APPEND _decimal_DEFINITIONS CONFIG_64)
    if(MSVC)
        list(APPEND _decimal_SOURCES _decimal/libmpdec/vcdiv64.asm)
    endif()
else()
    list(APPEND _decimal_DEFINITIONS CONFIG_32 ANSI)
endif()

ark_add_python_extension(_decimal SOURCES
    ${_decimal_SOURCES}
    BUILTIN
    DEFINITIONS Py_ENABLE_SHARED Py_BUILD_CORE_MODULE ${_decimal_DEFINITIONS}
    INCLUDEDIRS
        ${LOCAL_PYTHON_INCLUDE_DIRS}
        ${ARK_PYTHON_SRC_DIR}/Modules/_decimal
        ${ARK_PYTHON_SRC_DIR}/Modules/_decimal/libmpdec
)

ark_add_python_extension(_socket
    SOURCES socketmodule.c
    BUILTIN
    DEFINITIONS Py_ENABLE_SHARED Py_BUILD_CORE_MODULE
    INCLUDEDIRS
        ${LOCAL_PYTHON_INCLUDE_DIRS}
)

ark_add_python_extension(select
    SOURCES selectmodule.c
    BUILTIN
    DEFINITIONS Py_ENABLE_SHARED Py_BUILD_CORE_MODULE
    INCLUDEDIRS
        ${LOCAL_PYTHON_INCLUDE_DIRS}
)

ark_add_python_extension(pyexpat
    SOURCES pyexpat.c
    BUILTIN
    DEFINITIONS Py_ENABLE_SHARED Py_BUILD_CORE_MODULE
    INCLUDEDIRS
        ${LOCAL_PYTHON_INCLUDE_DIRS}
)

if(WIN32)
    ark_add_python_extension(overlapped
        SOURCES overlapped.c
        BUILTIN
        DEFINITIONS Py_ENABLE_SHARED Py_BUILD_CORE_MODULE
        INCLUDEDIRS
            ${LOCAL_PYTHON_INCLUDE_DIRS}
    )

    ark_link_libraries(Rpcrt4)
endif()
