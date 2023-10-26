message(STATUS "Loading CMakeLists.txt")

# The rest of your CMakeLists.txt content follows...
cmake_minimum_required(VERSION 3.10)

# Define the project
project(CVClient)


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(grpc_definitions.cmake)

# Find OpenCV
find_package(OpenCV REQUIRED)

# Include directories
include_directories(${OpenCV_INCLUDE_DIRS})

# Path to the Proto file
get_filename_component(cv_proto "../SharedComponents/grpc/CVService.proto" ABSOLUTE)
get_filename_component(cv_proto_path "${cv_proto}" PATH)

# Generated files
set(cv_proto_srcs "${cv_proto_path}/CVService.pb.cc")
set(cv_proto_hdrs "${cv_proto_path}/CVService.pb.h")
set(cv_grpc_srcs "${cv_proto_path}/CVService.grpc.pb.cc")
set(cv_grpc_hdrs "${cv_proto_path}/CVService.grpc.pb.h")

# Custom command to generate source and header files from Proto file
add_custom_command(
      OUTPUT "${cv_proto_srcs}" "${cv_proto_hdrs}" "${cv_grpc_srcs}" "${cv_grpc_hdrs}"
      COMMAND ${_PROTOBUF_PROTOC}
      ARGS --grpc_out "${cv_proto_path}"
        --cpp_out "${cv_proto_path}"
        -I "${cv_proto_path}"
        --plugin=protoc-gen-grpc="${_GRPC_CPP_PLUGIN_EXECUTABLE}"
        "${cv_proto}"
      DEPENDS "${cv_proto}")

# Include generated *.pb.h files
include_directories("${cv_proto_path}")

# Source files
set(SOURCE_FILES
    main.cpp
    VideoSource.cpp
    GRPCImageWriter.cpp
    ../SharedComponents/Logger.cpp
    ../SharedComponents/SafeQueue.cpp
    ../SharedComponents/configJson.cpp
    #"${cv_proto_path}/CVService.pb.cc"
    #"${cv_proto_path}/CVService.grpc.pb.cc"
   
)

# Header files (for clarity, even though they are not required for building an executable)
set(HEADER_FILES
    
    ICamera.hpp
    VideoSource.h
    GRPCImageWriter.h
    ImageWriter.hpp
    ../SharedComponents/structs.h
    ../SharedComponents/Logger.h
    ../SharedComponents/SafeQueue.h 
    ../SharedComponents/configJson.h
    #"${cv_proto_path}/CVService.pb.h"
    #"${cv_proto_path}/CVService.grpc.pb.h"

    
)

add_executable(CVClient ${SOURCE_FILES} ${HEADER_FILES})
set_target_properties(CVClient PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

#Create a new target that depends on the produced files
add_custom_target(
    GenerateProtobufSourcesClient
    DEPENDS "${cv_proto_srcs}" "${cv_proto_hdrs}" "${cv_grpc_srcs}" "${cv_grpc_hdrs}"
)
# Set the CVClient target to depend on the new target
add_dependencies(CVClient GenerateProtobufSourcesClient)

# Link libraries
target_link_libraries(CVClient ${OpenCV_LIBS})
target_link_libraries(CVClient #from common file
absl::flags
absl::flags_parse
${_REFLECTION}
${_GRPC_GRPCPP}
${_PROTOBUF_LIBPROTOBUF})

# On Windows, add the PROJECT_INCLUDES_PATH variable
if(WIN32)

    set(PROJECT_INCLUDES_PATH $ENV{PROJECT_INCLUDES_PATH})
    target_include_directories(CVClient PRIVATE ${PROJECT_INCLUDES_PATH})
    link_directories(${PROJECT_INCLUDES_PATH})
      # Add to runtime environment variables for debugging
    set_property(TARGET CVClient PROPERTY VS_DEBUGGER_ENVIRONMENT "PATH=${PROJECT_INCLUDES_PATH};$ENV{PATH}")

# Link sqlite3.lib   

 target_link_directories(CVClient PRIVATE ${PROJECT_INCLUDES_PATH})
endif()

if(UNIX AND NOT APPLE)
    find_package(spdlog REQUIRED)
    target_link_libraries(CVClient spdlog::spdlog)
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../SharedComponents/)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../SharedComponents/grpc/)

# Set the runtime output directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)


string(REGEX MATCH "([^;]*bin)$" BIN_PATH "$ENV{PROJECT_INCLUDES_PATH}")

if(WIN32 AND BIN_PATH)
    # Post-build command to copy OpenCV DLLs
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                       COMMAND ${CMAKE_COMMAND} -E copy_directory
                       "${BIN_PATH}" $<TARGET_FILE_DIR:${PROJECT_NAME}>)
endif()


