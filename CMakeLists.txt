cmake_minimum_required(VERSION 3.10)

project(OpenCVSolution) 

# Add the individual projects
add_subdirectory(CVClient)
add_subdirectory(CVServer)
add_subdirectory(OpenCVTest) # This is the new line to add the test project

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
