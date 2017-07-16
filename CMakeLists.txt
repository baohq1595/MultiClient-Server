cmake_minimum_required (VERSION 3.7.2)

#Needed to force cross compiling???
#set(CMAKE_TOOLCHAIN_FILE "Toolchain-RaspberryPi.cmake")

# Define project name
project (MultithreadTCPServer CXX C)

set(SUPPRESSED_WARNING "-Wno-conversion-null -Wno-write-strings")

# Compiling flags
set(CMAKE_CXX_FLAGS "-g")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${SUPPRESSED_WARNING}")

add_subdirectory(Server)
add_subdirectory(Client)

