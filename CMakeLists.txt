# CMakeLists.txt

# cmake needs this line
cmake_minimum_required(VERSION 2.8)

# Define project name
project(CreditNumberRecognizer)

# Version Number
set(serial "1.2.0")

#----------------------------------------
# Find OpenCV, you may need to set OpenCV_DIR variable
# to the absolute path to the directory containing OpenCVConfig.cmake file
# via the command line or GUI
find_package(OpenCV 3 REQUIRED core imgproc imgcodecs highgui)

# If the package has been found, several variables will
# be set, you can find the full list with descriptions
# in the OpenCVConfig.cmake file.

message(STATUS "OpenCV library status:")
message(STATUS "    version: ${OpenCV_VERSION}")
message(STATUS "    libraries: ${OpenCV_LIBS}")
message(STATUS "    include path: ${OpenCV_INCLUDE_DIRS}")


#--------------------------------------
# Find Boost
#set(Boost_USE_STATIC_LIBS        OFF)
set(Boost_USE_MULTITHREADED      ON)
#set(Boost_USE_STATIC_RUNTIME     OFF)
find_package(Boost 1.36 REQUIRED
             COMPONENTS filesystem program_options)
message("## Boost_INCLUDE_DIRS: ${Boost_INCLUDE_DIRS}")
message("## Boost_LIBRARY_DIRS: ${Boost_LIBRARY_DIRS}")

if(NOT Boost_FOUND)
    #   Config options
    set(BOOST_ROOT ${BOOST_ROOT} CACHE PATH "Set boost root directory" FORCE)
    set(Boost_LIBRARY_DIRS ${Boost_LIBRARY_DIRS} CACHE PATH "Set boost library directory" FORCE)
    set(Boost_USE_STATIC_LIBS ${Boost_USE_STATIC_LIBS} CACHE BOOL "Set use static library" FORCE)
    set(Boost_USE_STATIC_RUNTIME ${Boost_USE_STATIC_LIBS} CACHE BOOL "Set use static runtime" FORCE)
    message(FATAL_ERROR "Fail to find Boost")
endif()

#target_include_directories(CreditNumberRecognizer PUBLIC ${OpenCV_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS})
include_directories(${OpenCV_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS})

# Declare the executable target built from our sources
add_executable(CreditNumberRecognizer main.cpp MainAPI.cpp CreditNumberRecog.cpp common.cpp EdgeDirFeatures.cpp NumberDetect.cpp NumberRecog.cpp util.cpp)

set_target_properties(CreditNumberRecognizer PROPERTIES VERSION ${serial})

target_link_libraries(CreditNumberRecognizer ${OpenCV_LIBS} Boost::filesystem Boost::program_options)
