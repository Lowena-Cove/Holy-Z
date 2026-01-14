cmake_minimum_required(VERSION 3.10 FATAL_ERROR)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(HolyZ)

# Check for vcpkg integration
if(DEFINED CMAKE_TOOLCHAIN_FILE)
    message(STATUS "Using CMake toolchain: ${CMAKE_TOOLCHAIN_FILE}")
    if(CMAKE_TOOLCHAIN_FILE MATCHES "vcpkg")
        message(STATUS "vcpkg integration detected")
        set(USING_VCPKG TRUE)
    endif()
endif()

# If vcpkg is not being used, provide instructions
if(NOT USING_VCPKG)
    message(NOTICE "Holy-Z requires vcpkg for dependency management.")
    message(NOTICE "To build Holy-Z, please use vcpkg integration:")
    message(NOTICE "  1. Install vcpkg: https://github.com/Microsoft/vcpkg")
    message(NOTICE "  2. Run: cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE='<vcpkg-root>/scripts/buildsystems/vcpkg.cmake'")
    message(NOTICE "Or install dependencies manually and uncomment the fallback paths in CMakeLists.txt")
endif()

SET(programName Slang)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/CMakeFiles/")

# Source files for the HolyZ interpreter
set(HOLYZ_SOURCES
    Main.cpp
    strops.cpp
    eval.cpp
    system_control.cpp
)

# Header files
set(HOLYZ_HEADERS
    main.h
    anyops.h
    builtin.h
    strops.h
    eval.h
    graphics.h
    ZS.h
    color.hpp
    system_control.h
)

# For vcpkg, add the include directory for Boost headers BEFORE add_executable
if(DEFINED CMAKE_TOOLCHAIN_FILE)
    if(CMAKE_TOOLCHAIN_FILE MATCHES "vcpkg")
        # vcpkg installed Boost, add its include path
        set(VCPKG_INCLUDE "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/include")
        message(STATUS "Will use vcpkg include path: ${VCPKG_INCLUDE}")
    endif()
endif()

add_executable(HolyZ ${HOLYZ_SOURCES} ${HOLYZ_HEADERS})

# Apply include directories to target (this works for both old and newly generated projects)
if(DEFINED VCPKG_INCLUDE)
    target_include_directories(HolyZ PRIVATE "${VCPKG_INCLUDE}")
    message(STATUS "Applied vcpkg include path to HolyZ target")
endif()

# Boost Configuration (Optional but recommended for some features)
set(Boost_USE_STATIC_LIBS OFF) 
set(Boost_USE_MULTITHREADED ON)  
set(Boost_USE_STATIC_RUNTIME OFF) 

option(HOLYZ_ENABLE_BOOST "Enable Boost libraries" ON)

if(HOLYZ_ENABLE_BOOST)
    # Try to find Boost using vcpkg first (modern CMake targets)
    find_package(Boost 1.70 CONFIG QUIET)
    if(Boost_FOUND)
        message(STATUS "✓ Found Boost via vcpkg CONFIG at ${Boost_DIR}")
        # vcpkg provides Boost::boost, Boost::system and Boost::filesystem targets
        target_link_libraries(HolyZ PUBLIC Boost::boost)
        if(TARGET Boost::system)
            target_link_libraries(HolyZ PUBLIC Boost::system)
        endif()
        if(TARGET Boost::filesystem)
            target_link_libraries(HolyZ PUBLIC Boost::filesystem)
        endif()
    else()
        message(NOTICE "Boost CONFIG module not found, trying MODULE mode...")
        # Try MODULE mode as fallback
        find_package(Boost 1.70 QUIET COMPONENTS system filesystem)
        if(Boost_FOUND)
            message(STATUS "✓ Found Boost via MODULE mode")
            include_directories(${Boost_INCLUDE_DIRS})
            target_link_libraries(HolyZ ${Boost_LIBRARIES})
        else()
            message(STATUS "⚠ Boost headers are included but CMake targets not found. Compilation will still work.")
            message(STATUS "  If linking fails, install Boost: vcpkg install boost:x64-windows")
            message(STATUS "  To disable this warning: -DHOLYZ_ENABLE_BOOST=OFF")
        endif()
    endif()
else()
    message(STATUS "✓ Building without explicit Boost CMake targets (headers assumed to be in path)")
    message(STATUS "  To enable Boost targets: cmake -DHOLYZ_ENABLE_BOOST=ON")
endif()

# SDL2 Configuration (Optional for graphics features)
# Note: SDL2 is optional. The core system_control functions work without it.
# To build with graphics support, install SDL2 via vcpkg or manually.

option(HOLYZ_ENABLE_GRAPHICS "Enable SDL2-based graphics support" OFF)

if(HOLYZ_ENABLE_GRAPHICS)
    # Try to find SDL2 using vcpkg first (modern CMake targets)
    find_package(SDL2 CONFIG QUIET)
    if(SDL2_FOUND)
        # vcpkg provides SDL2::SDL2 and SDL2::SDL2main targets
        message(STATUS "✓ Found SDL2 via vcpkg CONFIG")
        target_link_libraries(HolyZ SDL2::SDL2 SDL2::SDL2main)
    else()
        # Fallback to custom find modules
        find_package(SDL2 QUIET MODULE)
        if(SDL2_FOUND)
            message(STATUS "✓ Found SDL2 via custom find module")
            include_directories(${SDL2_INCLUDE_DIRS})
            target_link_libraries(HolyZ ${SDL2_LIBRARIES})
        else()
            message(WARNING "SDL2 not found. Graphics features will not be available. "
                          "Install SDL2 via vcpkg or use: -DHOLYZ_ENABLE_GRAPHICS=OFF")
        endif()
    endif()

    # SDL2_image Configuration (Optional)
    find_package(SDL2_image CONFIG QUIET)
    if(SDL2_image_FOUND)
        message(STATUS "✓ Found SDL2_image via vcpkg CONFIG")
        target_link_libraries(HolyZ SDL2_image::SDL2_image)
    else()
        find_package(SDL2_image QUIET MODULE)
        if(SDL2_image_FOUND)
            message(STATUS "✓ Found SDL2_image via custom find module")
            include_directories(${SDL2_IMAGE_INCLUDE_DIRS})
            target_link_libraries(HolyZ ${SDL2_IMAGE_LIBRARIES})
        else()
            message(STATUS "⚠ SDL2_image not found. Image support will not be available.")
        endif()
    endif()

    # SDL2_ttf Configuration (Optional)
    find_package(SDL2_ttf CONFIG QUIET)
    if(SDL2_ttf_FOUND)
        message(STATUS "✓ Found SDL2_ttf via vcpkg CONFIG")
        target_link_libraries(HolyZ SDL2_ttf::SDL2_ttf)
    else()
        find_package(SDL2_ttf QUIET MODULE)
        if(SDL2_ttf_FOUND)
            message(STATUS "✓ Found SDL2_ttf via custom find module")
            include_directories(${SDL2_TTF_INCLUDE_DIRS})
            target_link_libraries(HolyZ ${SDL2_TTF_LIBRARIES})
        else()
            message(STATUS "⚠ SDL2_ttf not found. Text rendering will not be available.")
        endif()
    endif()
else()
    message(STATUS "✓ Building without SDL2 graphics support (core functions available)")
    message(STATUS "  To enable graphics: cmake -DHOLYZ_ENABLE_GRAPHICS=ON")
endif()

# Additional fallback include and library directories for manual SDL2 installation
# Uncomment and adjust paths as needed for your SDL2 installation
# include_directories("D:/Code/SDL2_ttf-2.0.15/include")
# include_directories("D:/Code/SDL2-2.0.18/include")
# include_directories("D:/Code/SDL2_image-2.0.5/include")
# include_directories("D:/Code/boost")

# link_directories("D:/Code/SDL2_ttf-2.0.15/lib/x64")
# link_directories("D:/Code/SDL2-2.0.18/lib/x64")
# link_directories("D:/Code/SDL2_image-2.0.5/lib/x64")
# link_directories("D:/Code/boost")

