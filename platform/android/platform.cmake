
#cmake -G "Ninja" -DCMAKE_TOOLCHAIN_FILE=..\${ANDROID_NDK}\android.toolchain.cmake -DANDROID_NATIVE_API_LEVEL="android-18" -DANDROID_ABI="armeabi-v7a with NEON" -DBUILD_SHARED_LIBS="OFF" -DCMAKE_BUILD_TYPE="Release" -DCMAKE_DEBUG_POSFIX="" -DANDROID_STL=gnustl_shared ..\..

add_definitions(-DLOG_TAG="ark")
add_definitions(-DHAVE_MALLOC)
add_definitions(-DGL_GLEXT_PROTOTYPES=1)
add_definitions(-DXML_DEV_URANDOM)

set(PTHREADS OFF)

aux_source_directory(platform/android/impl LOCAL_SRC_LIST)
aux_source_directory(platform/android/impl/application LOCAL_SRC_LIST)
aux_source_directory(platform/android/impl/readable LOCAL_SRC_LIST)
aux_source_directory(platform/android/impl/resource LOCAL_SRC_LIST)
aux_source_directory(platform/android/util LOCAL_SRC_LIST)

list(APPEND LOCAL_LIBS log android GLESv3)

add_library(pthread platform/android/impl/dummy_pthread.cpp)

if(ARK_USE_VULKAN)
    ark_compile_definitions(-DVK_USE_PLATFORM_ANDROID_KHR)
    aux_source_directory(platform/android/platform/vulkan LOCAL_SRC_LIST)
endif()
