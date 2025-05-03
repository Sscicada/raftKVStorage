#ifndef RAFT_RPC_H
#define RAFT_RPC_H

#include <memory>

#include "raft.pb.h"
#include "raft.grpc.pb.h" // 生成RaftRpcService::Stub
#include <grpcpp/grpcpp.h>

/*
    作为客户端，主动调用服务端的 RPC 方法
*/
class RaftRPC {
public:
    // // 初始化到特定地址的gRPC连接
    // RaftRPC(const std::string& server_address) {
    //     channel_ = grpc::CreateChannel(
    //         server_address, 
    //         grpc::InsecureChannelCredentials()
    //     );
    //     stub_ = RaftRpcService::NewStub(channel_);
    // }

    // RaftRpc(std::string ip, short port);

    // 领导者向其他节点发送日志复制和心跳消息
    void sendAppendEntries(int targetId, const AppendEntriesRequest& req);

    // 向其他节点发送投票请求，实现选举
    void sendRequestVote(int targetId, const RequestVoteRequest& req);

    // 将一个 RaftNode 对象注册到 RaftRPC 中，通常是将 RaftNode 作为一个回调或者消息接收对象
    // 为了让 RaftRPC 知道该如何向 RaftNode 发送消息或接收消息。
    void registerRaftNode(RaftNode* node);

    // 当 RaftRPC 收到一个 AppendEntries 请求时，调用该方法进行处理
    void onReceiveAppendEntries(const AppendEntriesRequest& req);

    // 当 RaftRPC 收到一个选举请求 RequestVote 时，调用该方法进行处理
    void onReceiveRequestVote(const RequestVoteRequest& req);

private:
    // 由 protobuf 生成的客户端存根，提供具体的 RPC 方法调用接口
    std::unique_ptr<RaftRpcService::Stub> stub_;
};

#endif // RAFT_RPC_H