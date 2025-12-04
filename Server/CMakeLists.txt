cmake_minimum_required(VERSION 3.24)

project(OCRServer CXX)
set(CMAKE_CXX_STANDARD 20)

find_package(Threads REQUIRED)
find_package(Protobuf REQUIRED)
find_package(PkgConfig REQUIRED) 

find_package(gRPC CONFIG)
if (NOT gRPC_FOUND)
    pkg_check_modules(gRPC REQUIRED grpc++ grpc)
endif()

find_package(Tesseract CONFIG)
if (NOT Tesseract_FOUND)
    pkg_check_modules(Tesseract REQUIRED tesseract)
endif()

find_package(Leptonica CONFIG)
if (NOT Leptonica_FOUND)
    pkg_check_modules(Leptonica REQUIRED lept)
endif()

# grpc plugins setup
set(_PROTOBUF_LIBPROTOBUF protobuf::libprotobuf)
set(_REFLECTION_grpc++_reflection grpc::grpc++_reflection)
find_program(_PROTOBUF_PROTOC protoc)
find_program(_GRPC_CPP_PLUGIN_EXECUTABLE grpc_cpp_plugin)

if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../protos/ocr.proto")
    get_filename_component(PROTO_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../protos" ABSOLUTE)
else()
    get_filename_component(PROTO_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/.." ABSOLUTE)
endif()

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

add_executable(OCRServer
    main_server.cpp
    ${hw_proto_srcs}
    ${hw_grpc_srcs}
)

target_include_directories(OCRServer PRIVATE 
    "${CMAKE_CURRENT_BINARY_DIR}"
    ${Tesseract_INCLUDE_DIRS} 
    ${Leptonica_INCLUDE_DIRS}
    ${gRPC_INCLUDE_DIRS}
)

set(FINAL_LIBS "")

if (TARGET gRPC::grpc++)
    list(APPEND FINAL_LIBS gRPC::grpc++ gRPC::grpc++_reflection)
else()
    list(APPEND FINAL_LIBS ${gRPC_LIBRARIES})
    list(APPEND FINAL_LIBS grpc++_reflection) 
endif()

if (TARGET Tesseract::libtesseract)
    list(APPEND FINAL_LIBS Tesseract::libtesseract)
else()
    list(APPEND FINAL_LIBS ${Tesseract_LIBRARIES})
endif()

if (TARGET Leptonica::libleptonica)
    list(APPEND FINAL_LIBS Leptonica::libleptonica)
elseif (TARGET leptonica)
    list(APPEND FINAL_LIBS leptonica)
else()
    list(APPEND FINAL_LIBS ${Leptonica_LIBRARIES})
endif()

target_link_libraries(OCRServer PRIVATE
    protobuf::libprotobuf
    Threads::Threads
    ${FINAL_LIBS}
)