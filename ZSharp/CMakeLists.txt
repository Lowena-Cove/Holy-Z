cmake_minimum_required(VERSION 3.10 FATAL_ERROR)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(HolyZ)

SET(programName Slang)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/CMakeFiles/")

add_executable (HolyZ Main.cpp main.h anyops.h builtin.h strops.cpp strops.h eval.cpp eval.h graphics.h ZS.h color.hpp)

# Try to find SDL2 using vcpkg first (modern CMake targets)
find_package(SDL2 CONFIG QUIET)
if(SDL2_FOUND)
    # vcpkg provides SDL2::SDL2 and SDL2::SDL2main targets
    set(SDL2_LIBRARIES SDL2::SDL2 SDL2::SDL2main)
    get_target_property(SDL2_INCLUDE_DIRS SDL2::SDL2 INTERFACE_INCLUDE_DIRECTORIES)
else()
    # Fallback to custom find modules
    find_package(SDL2 REQUIRED)
endif()

find_package(SDL2_image CONFIG QUIET)
if(NOT SDL2_image_FOUND)
    find_package(SDL2_image REQUIRED)
    set(SDL2_IMAGE_LIBRARIES ${SDL2_IMAGE_LIBRARIES})
else()
    set(SDL2_IMAGE_LIBRARIES SDL2_image::SDL2_image)
endif()

find_package(SDL2_ttf CONFIG QUIET)
if(NOT SDL2_ttf_FOUND)
    find_package(SDL2_ttf REQUIRED)
    set(SDL2_TTF_LIBRARIES ${SDL2_TTF_LIBRARIES})
else()
    set(SDL2_TTF_LIBRARIES SDL2_ttf::SDL2_ttf)
endif()

include_directories(${SDL2_INCLUDE_DIRS} ${SDL2_IMAGE_INCLUDE_DIRS} ${SDL2_TTF_INCLUDE_DIRS})
# include_directories("D:/Code/SDL2_ttf-2.0.15/include")
# include_directories("D:/Code/SDL2-2.0.18/include")
# include_directories("D:/Code/SDL2_image-2.0.5/include")
# include_directories("D:/Code/boost")

# link_directories("D:/Code/SDL2_ttf-2.0.15/lib/x64")
# link_directories("D:/Code/SDL2-2.0.18/lib/x64")
# link_directories("D:/Code/SDL2_image-2.0.5/lib/x64")
# link_directories("D:/Code/boost")

set(Boost_USE_STATIC_LIBS OFF) 
set(Boost_USE_MULTITHREADED ON)  
set(Boost_USE_STATIC_RUNTIME OFF) 

# Try to find Boost using vcpkg first (modern CMake targets)
find_package(Boost CONFIG QUIET COMPONENTS system filesystem)
if(Boost_FOUND)
    # vcpkg provides Boost::system and Boost::filesystem targets
    set(Boost_LIBRARIES Boost::system Boost::filesystem)
else()
    # Fallback to traditional FindBoost
    FIND_PACKAGE(Boost REQUIRED COMPONENTS system filesystem)
endif()

include_directories(${Boost_INCLUDE_DIRS}) 

target_link_libraries(HolyZ m ${Boost_LIBRARIES} ${SDL2_LIBRARIES} ${SDL2_IMAGE_LIBRARIES} ${SDL2_TTF_LIBRARIES})
