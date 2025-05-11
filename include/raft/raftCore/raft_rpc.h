#ifndef RAFT_RPC_H
#define RAFT_RPC_H

#include <memory>

#include "raft.pb.h"

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
    void sendAppendEntries(int targetId, const raft::AppendEntriesRequest& req);

    // 向其他节点发送投票请求，实现选举
    void sendRequestVote(int targetId, const raft::RequestVoteRequest& req);

private:
    // 由 protobuf 生成的客户端存根，提供具体的 RPC 方法调用接口
    std::unique_ptr<raft::RaftRPCService::Stub> stub_;
};

#endif // RAFT_RPC_H