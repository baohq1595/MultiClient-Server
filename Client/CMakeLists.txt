cmake_minimum_required (VERSION 2.6)

# Define project name
project (Client)

#Search for Boost Libraries - STATIC only
set(Boost_USE_STATIC_LIBS ON)
find_package(Boost 1.63.0 REQUIRED program_options thread)

#Search for OpenCV
find_package(OpenCV REQUIRED)

set(LIBRARIES ${Boost_LIBRARIES} ${OpenCV_LIBS} rt m pthread gcov)

# Add include directories
include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR} ${Boost_INCLUDE_DIRS} ${OpenCV_INCLUDE_DIRS})

link_directories(${Boost_LIBRARY_DIRS})


set ( Client_SRCS
    mainClient.cpp
)
add_executable(Client ${Client_SRCS})
target_link_libraries(Client ${LIBRARIES})
