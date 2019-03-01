# - Find fmod
# Find the fmod includes and library
#
#  FMOD_INCLUDE_DIR - Where to find fmod includes
#  FMOD_LIBRARIES   - List of libraries when using fmod
#  FMOD_FOUND       - True if fmod was found

IF(FMOD_INCLUDE_DIR)
  SET(FMOD_FIND_QUIETLY TRUE)
ENDIF()

FIND_PATH(FMOD_INCLUDE_DIR "fmod_studio.hpp"
  PATHS
  $ENV{FMOD_HOME}/inc
  $ENV{FMOD_HOME}/api/studio/inc
  $ENV{EXTERNLIBS}/fmod/studio/inc
  ~/Library/Frameworks
  /Library/Frameworks
  /usr/local
  /usr
  /sw # Fink
  /opt/local # DarwinPorts
  /opt/csw # Blastwave
  /opt
  DOC "fmod - Headers"
  CMAKE_FIND_ROOT_PATH_BOTH
)
FIND_PATH(FMOD_LL_INCLUDE_DIR "fmod.h"
  PATHS
  $ENV{FMOD_HOME}/inc
  $ENV{FMOD_HOME}/api/lowlevel/inc
  $ENV{EXTERNLIBS}/fmod/lowlevel/inc
  ~/Library/Frameworks
  /Library/Frameworks
  /usr/local
  /usr
  /sw # Fink
  /opt/local # DarwinPorts
  /opt/csw # Blastwave
  /opt
  DOC "fmod - Headers"
  CMAKE_FIND_ROOT_PATH_BOTH
)

IF(MSVC)
    IF(CMAKE_SIZEOF_VOID_P EQUAL 8)
        SET(FMOD_EVENT_NAMES fmod64_vc)
        SET(FMOD_EVENT_DBG_NAMES fmod64_vc)
    ELSE()
        SET(FMOD_EVENT_NAMES fmod_vc)
        SET(FMOD_EVENT_DBG_NAMES fmod_vc)
    ENDIF()
ELSE()
    SET(FMOD_NAMES fmodstudio)
    SET(FMOD_DBG_NAMES fmodstudioL)
    SET(FMOD_EVENT_NAMES fmodL)
    SET(FMOD_EVENT_DBG_NAMES fmodL)
ENDIF()

IF(ANDROID)
    FIND_LIBRARY(FMOD_LIBRARY NAMES ${FMOD_NAMES}
      PATHS
      $ENV{FMOD_HOME}/api/studio/lib/armeabi-v7a
      $ENV{EXTERNLIBS}/fmod/studio/lib
      PATH_SUFFIXES lib lib64
      DOC "fmod - Library"
      CMAKE_FIND_ROOT_PATH_BOTH
    )

    FIND_LIBRARY(FMOD_EVENT_LIBRARY NAMES ${FMOD_EVENT_NAMES}
      PATHS
      $ENV{FMOD_HOME}/api/lowlevel/lib/armeabi-v7a
      $ENV{EXTERNLIBS}/fmod/lowlevel/lib
      DOC "fmod_event - Library"
      CMAKE_FIND_ROOT_PATH_BOTH
    )
ELSE()
    FIND_LIBRARY(FMOD_LIBRARY NAMES ${FMOD_NAMES}
      PATHS
      $ENV{FMOD_HOME}
      $ENV{FMOD_HOME}/api/studio/lib
      $ENV{EXTERNLIBS}/fmod/studio/lib
      ~/Library/Frameworks
      /Library/Frameworks
      /usr/local
      /usr
      /sw
      /opt/local
      /opt/csw
      /opt
      PATH_SUFFIXES lib lib64
      DOC "fmod - Library"
      CMAKE_FIND_ROOT_PATH_BOTH
    )
    FIND_LIBRARY(FMOD_EVENT_LIBRARY NAMES ${FMOD_EVENT_NAMES}
      PATHS
      $ENV{FMOD_HOME}
      $ENV{FMOD_HOME}/api/lowlevel/lib
      $ENV{EXTERNLIBS}/fmod/lowlevel/lib
      ~/Library/Frameworks
      /Library/Frameworks
      /usr/local
      /usr
      /sw
      /opt/local
      /opt/csw
      /opt
      PATH_SUFFIXES lib lib64
      DOC "fmod_event - Library"
      CMAKE_FIND_ROOT_PATH_BOTH
    )
ENDIF()

INCLUDE(FindPackageHandleStandardArgs)

IF(MSVC)
  # VisualStudio needs a debug version
  FIND_LIBRARY(FMOD_LIBRARY_DEBUG NAMES ${FMOD_DBG_NAMES}
    PATHS
    $ENV{FMOD_HOME}
    $ENV{FMOD_HOME}/api/studio/lib
    $ENV{EXTERNLIBS}/fmod/studio/lib
    PATH_SUFFIXES lib lib64
    DOC "fmod - Library (Debug)"
  )
  FIND_LIBRARY(FMOD_EVENT_LIBRARY_DEBUG NAMES ${FMOD_EVENT_DBG_NAMES}
    PATHS
    $ENV{FMOD_HOME}
    $ENV{FMOD_HOME}/api/lowlevel/lib
    $ENV{EXTERNLIBS}/fmod/lowlevel/lib
    PATH_SUFFIXES lib lib64
    DOC "fmod_event - Library (Debug)"
  )

  IF(FMOD_LIBRARY_DEBUG AND FMOD_LIBRARY)
    SET(FMOD_LIBRARIES optimized ${FMOD_LIBRARY} debug ${FMOD_LIBRARY_DEBUG} optimized ${FMOD_EVENT_LIBRARY} debug ${FMOD_EVENT_LIBRARY_DEBUG})
  ENDIF(FMOD_LIBRARY_DEBUG AND FMOD_LIBRARY)

  FIND_PACKAGE_HANDLE_STANDARD_ARGS(FMOD DEFAULT_MSG FMOD_LIBRARY FMOD_LIBRARY_DEBUG FMOD_INCLUDE_DIR)

  MARK_AS_ADVANCED(FMOD_LIBRARY FMOD_LIBRARY_DEBUG FMOD_INCLUDE_DIR)

ELSE(MSVC)
  # rest of the world
  SET(FMOD_LIBRARIES ${FMOD_LIBRARY} ${FMOD_EVENT_LIBRARY})

  FIND_PACKAGE_HANDLE_STANDARD_ARGS(FMOD DEFAULT_MSG FMOD_LIBRARY FMOD_INCLUDE_DIR)

  MARK_AS_ADVANCED(FMOD_LIBRARY FMOD_INCLUDE_DIR)

ENDIF(MSVC)

IF(FMOD_FOUND)
  SET(FMOD_INCLUDE_DIRS ${FMOD_INCLUDE_DIR} ${FMOD_LL_INCLUDE_DIR})
ENDIF(FMOD_FOUND)
