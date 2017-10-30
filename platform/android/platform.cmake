
#cmake -G "Ninja" -DCMAKE_TOOLCHAIN_FILE=${ARK_SRC_DIR}\platform\android\make\android.toolchain.cmake -DANDROID_NATIVE_API_LEVEL="android-9" -DANDROID_ABI="armeabi-v7a" -DBUILD_SHARED_LIBS="OFF" -DCMAKE_BUILD_TYPE="Debug" ..\..

add_definitions(-DLOG_TAG="ark")
add_definitions(-DPY_FORMAT_LONG_LONG="long long")
add_definitions(-DHAVE_MALLOC)
add_definitions(-DGL_GLEXT_PROTOTYPES=1)

set(PTHREADS OFF)

aux_source_directory(platform/android/impl LOCAL_SRC_LIST)
aux_source_directory(platform/android/impl/application LOCAL_SRC_LIST)
aux_source_directory(platform/android/impl/readable LOCAL_SRC_LIST)
aux_source_directory(platform/android/impl/resource LOCAL_SRC_LIST)
aux_source_directory(platform/android/util LOCAL_SRC_LIST)

list(APPEND LOCAL_LIBS log android GLESv3)

add_library(pthread platform/android/impl/dummy_pthread.cpp)
