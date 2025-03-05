cmake_minimum_required(VERSION 3.20)
project(DistributedFileNetwork)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)

find_package(Protobuf CONFIG REQUIRED)
find_package(gRPC CONFIG REQUIRED)

set(PROTO_FILES 
    proto/client.proto
    proto/storage.proto
    proto/tracking.proto
)

get_target_property(grpc_cpp_plugin_location gRPC::grpc_cpp_plugin LOCATION)

protobuf_generate(
    LANGUAGE cpp
    OUT_VAR PROTO_SRCS
    PROTOS ${PROTO_FILES}
)

protobuf_generate(
    LANGUAGE grpc
    GENERATE_EXTENSIONS .grpc.pb.h .grpc.pb.cc
    PLUGIN "protoc-gen-grpc=${grpc_cpp_plugin_location}"
    OUT_VAR GRPC_SRCS
    PROTOS ${PROTO_FILES}
)

set(GENERATED_SRCS ${PROTO_SRCS} ${GRPC_SRCS})

add_library(protolib ${GENERATED_SRCS})
target_link_libraries(protolib PUBLIC gRPC::grpc++ protobuf::libprotobuf)
target_include_directories(protolib PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

add_executable(main src/main.cc)
target_link_libraries(main PRIVATE protolib gRPC::grpc++ protobuf::libprotobuf)
