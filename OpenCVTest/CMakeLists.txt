cmake_minimum_required(VERSION 3.14)
project(OpenCVTest)

# Set C++ standard
set(CMAKE_CXX_STANDARD 14)

# Find OpenCV
find_package(OpenCV REQUIRED)

# Include directories
include_directories(${OpenCV_INCLUDE_DIRS})

# Source files for the test project
set(TEST_SOURCE_FILES
    test.cpp
    pch.cpp
    # Add any other test source files here
)

# Header files
set(TEST_HEADER_FILES
    pch.h
    # Add your test header files here if any
)

# If on Windows, use FetchContent to get GoogleTest
if(WIN32)
    include(FetchContent)
    FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
    )
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)
else()
    find_package(GTest REQUIRED)
    include_directories(${GTEST_INCLUDE_DIRS})
endif()

# Create the test executable
add_executable(OpenCVTest ${TEST_SOURCE_FILES} ${TEST_HEADER_FILES} packages.config)
set_target_properties(OpenCVTest PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Add the POST_BUILD custom command here
add_custom_command(
    TARGET OpenCVTest
    POST_BUILD
    COMMAND exit 0
    VERBATIM
)

# On Windows, add the PROJECT_INCLUDES_PATH variable
if(WIN32)
    set(PROJECT_INCLUDES_PATH $ENV{PROJECT_INCLUDES_PATH})
    target_include_directories(OpenCVTest PRIVATE ${PROJECT_INCLUDES_PATH})
    link_directories(${PROJECT_INCLUDES_PATH})
    # Add to runtime environment variables for debugging
    set_property(TARGET OpenCVTest PROPERTY VS_DEBUGGER_ENVIRONMENT "PATH=${PROJECT_INCLUDES_PATH};$ENV{PATH}")
    # Link sqlite3.lib   
    target_link_libraries(OpenCVTest sqlite3.lib)
    target_link_directories(OpenCVTest PRIVATE ${PROJECT_INCLUDES_PATH})
endif()

if(UNIX AND NOT APPLE)
    find_package(SQLite3 REQUIRED)
    target_link_libraries(OpenCVProject ${SQLite3_LIBRARIES})
    find_package(spdlog REQUIRED)
    target_link_libraries(OpenCVProject spdlog::spdlog)

endif()


# Link libraries
if(WIN32)
    target_link_libraries(OpenCVTest ${OpenCV_LIBS} gtest gtest_main)
else()
    target_link_libraries(OpenCVTest ${OpenCV_LIBS} ${GTEST_LIBRARIES} pthread)
endif()

# Set the runtime output directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Add the test to CTest
include(GoogleTest)
gtest_discover_tests(OpenCVTest)


