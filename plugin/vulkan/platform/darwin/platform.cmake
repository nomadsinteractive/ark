ark_find_vcpkg_package(Threads MODULE_MODE LIBRARIES Threads::Threads)
ark_compile_definitions(-DVK_USE_PLATFORM_MACOS_MVK)

ark_find_and_link_libraries(QuartzCore Metal IOSurface)

if(IOS)
    ark_find_and_link_libraries(UIKit AudioToolbox AVFoundation)
endif()
