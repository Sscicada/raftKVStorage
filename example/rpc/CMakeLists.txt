set(PROTO_FILES
    ${PROJECT_SOURCE_DIR}/proto/friend.proto
)

message(STATUS "friend PROTO_FILES=${PROTO_FILES}")  # 打印路径确认

# 编译 proto 文件，生成相应的 C++ 头文件和源文件
# protobuf_generate_cpp 只有在和 add_executable() 或 add_library() 同一目录中才会正常工作。
protobuf_generate_cpp(PROTO_SRC PROTO_HDR ${PROTO_FILES})

message("编译 firend.proto 文件")
message("PROTO_SRC=${PROTO_SRC}")  # 显示生成的 .pb.cc 文件路径
message("PROTO_HDR=${PROTO_HDR}")  # 显示生成的 .pb.h 文件路径

link_directories(${PROJECT_SOURCE_DIR}/third_party/muduo/lib)

set(SRC_LIST rpc_callee.cpp ${PROTO_SRC})

add_executable(provider ${SRC_LIST})

target_link_libraries(provider rpc protobuf muduo_net muduo_base pthread)

set(SRC_LIST rpc_caller.cpp ${PROTO_SRC})

add_executable(consumer ${SRC_LIST})

target_link_libraries(consumer rpc protobuf)