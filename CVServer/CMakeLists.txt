cmake_minimum_required(VERSION 3.10)
project(CVServer)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# file with grpc prop
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
    YoloBackend.cpp
    ../SharedComponents/Logger.cpp
    ../SharedComponents/SafeQueue.cpp
    SqlDB.cpp
    GRPCImageReader.cpp
    ../SharedComponents/grpc/CVService.pb.cc
    ../SharedComponents/grpc/CVService.grpc.pb.cc
    ../SharedComponents/configJson.cpp
)

# Header files (for clarity, even though they are not required for building an executable)
set(HEADER_FILES
    IBackend.hpp
    yoloBackend.h
    ../SharedComponents/structs.h
    ../SharedComponents/Logger.h
    ../SharedComponents/SafeQueue.h 
    ../SharedComponents/configJson.h
    SqlDB.h
    ImageReader.hpp
    GRPCImageReader.h
    #../SharedComponents/grpc/CVService.pb.h
    #../SharedComponents/grpc/CVService.grpc.pb.h
)

add_executable(CVServer ${SOURCE_FILES} ${HEADER_FILES})
set_target_properties(CVServer PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

#Create a new target that depends on the produced files
add_custom_target(
    GenerateProtobufSourcesServer
    DEPENDS "${cv_proto_srcs}" "${cv_proto_hdrs}" "${cv_grpc_srcs}" "${cv_grpc_hdrs}"
)
# Set the CVServer target to depend on the new target
add_dependencies(CVServer GenerateProtobufSourcesServer)


# On Windows, add the PROJECT_INCLUDES_PATH variable
if(WIN32)
    set(PROJECT_INCLUDES_PATH $ENV{PROJECT_INCLUDES_PATH})
    target_include_directories(CVServer PRIVATE ${PROJECT_INCLUDES_PATH})
    link_directories(${PROJECT_INCLUDES_PATH})
      # Add to runtime environment variables for debugging
    set_property(TARGET CVServer PROPERTY VS_DEBUGGER_ENVIRONMENT "PATH=${PROJECT_INCLUDES_PATH};$ENV{PATH}")

# Link sqlite3.lib   
 target_link_libraries(CVServer sqlite3.lib)
 target_link_directories(CVServer PRIVATE ${PROJECT_INCLUDES_PATH})
endif()

if(UNIX AND NOT APPLE)
    find_package(SQLite3 REQUIRED)
    target_link_libraries(CVServer ${SQLite3_LIBRARIES})
    find_package(spdlog REQUIRED)
    target_link_libraries(CVServer spdlog::spdlog)

    message(STATUS "OpenCV version: ${OpenCV_VERSION}")

endif()



# Link libraries
target_link_libraries(CVServer ${OpenCV_LIBS})
target_link_libraries(CVServer #from grpc file
absl::flags
absl::flags_parse
${_REFLECTION}
${_GRPC_GRPCPP}
${_PROTOBUF_LIBPROTOBUF})

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