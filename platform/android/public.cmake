set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -mfloat-abi=softfp")

ark_link_libraries(android)

ark_compile_definitions(-DARK_PLATFORM_ANDROID)

ark_include_directories(${ANDROID_NDK}/sources/android/native_app_glue)
