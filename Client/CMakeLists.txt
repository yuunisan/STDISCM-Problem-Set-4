cmake_minimum_required(VERSION 3.24)

project(OCRClient CXX)
set(CMAKE_CXX_STANDARD 20)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# finding the needed pakcages
find_package(Qt6 REQUIRED COMPONENTS Widgets Core Gui)
find_package(protobuf CONFIG REQUIRED)
find_package(gRPC CONFIG REQUIRED)
find_package(Threads REQUIRED)

# grpc required pluginss
set(_PROTOBUF_LIBPROTOBUF protobuf::libprotobuf)
set(_REFLECTION_grpc++_reflection grpc::grpc++_reflection)
find_program(_PROTOBUF_PROTOC protoc)
find_program(_GRPC_CPP_PLUGIN_EXECUTABLE grpc_cpp_plugin)

# pathing to protos file
get_filename_component(PROTO_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../protos" ABSOLUTE)
set(HW_PROTO "${PROTO_SRC_DIR}/ocr.proto")
get_filename_component(hw_proto_path "${HW_PROTO}" ABSOLUTE)
get_filename_component(hw_proto_name "${HW_PROTO}" NAME_WE)

set(hw_proto_srcs "${CMAKE_CURRENT_BINARY_DIR}/${hw_proto_name}.pb.cc")
set(hw_proto_hdrs "${CMAKE_CURRENT_BINARY_DIR}/${hw_proto_name}.pb.h")
set(hw_grpc_srcs "${CMAKE_CURRENT_BINARY_DIR}/${hw_proto_name}.grpc.pb.cc")
set(hw_grpc_hdrs "${CMAKE_CURRENT_BINARY_DIR}/${hw_proto_name}.grpc.pb.h")

add_custom_command(
      OUTPUT "${hw_proto_srcs}" "${hw_proto_hdrs}" "${hw_grpc_srcs}" "${hw_grpc_hdrs}"
      COMMAND ${_PROTOBUF_PROTOC}
      ARGS --grpc_out "${CMAKE_CURRENT_BINARY_DIR}"
           --cpp_out "${CMAKE_CURRENT_BINARY_DIR}"
           -I "${PROTO_SRC_DIR}"
           --plugin=protoc-gen-grpc="${_GRPC_CPP_PLUGIN_EXECUTABLE}"
           "${hw_proto_path}"
      DEPENDS "${hw_proto_path}")

# client exec
add_executable(OCRClient
    main.cpp
    MainWindow.h
    MainWindow.cpp
    ${hw_proto_srcs}
    ${hw_grpc_srcs}
)

# include directory of ocrclient
target_include_directories(OCRClient PRIVATE "${CMAKE_CURRENT_BINARY_DIR}")

# link to the libraries used in guiwindow, and grpc communications
target_link_libraries(OCRClient PRIVATE
    Qt6::Widgets
    Qt6::Core
    Qt6::Gui
    protobuf::libprotobuf
    gRPC::grpc++
    gRPC::grpc++_reflection
    Threads::Threads
)