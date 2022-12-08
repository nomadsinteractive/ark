ark_setup_tools(ark_add_python_extension)

ark_find_vcpkg_package(libffi LIBRARIES libffi)
ark_add_python_extension(_ctypes
          SOURCES _ctypes/_ctypes.c
                  _ctypes/callbacks.c
                  _ctypes/callproc.c
                  _ctypes/cfield.c
                  _ctypes/malloc_closure.c
                  _ctypes/stgdict.c
          BUILTIN
          INCLUDEDIRS
              ${ARK_PYTHON_INCLUDE_DIR} ${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/include ${ARK_PYTHON_SRC_DIR}/Include/internal
          DEFINITIONS Py_ENABLE_SHARED Py_BUILD_CORE_MODULE FFI_BUILDING
          LIBRARIES libffi ${ARK_PYTHON_LIBRARIES}
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
        ${ARK_PYTHON_INCLUDE_DIR}
        ${ARK_PYTHON_SRC_DIR}/Modules/_decimal
        ${ARK_PYTHON_SRC_DIR}/Modules/_decimal/libmpdec
)