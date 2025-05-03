#ifndef RAFT_NODE_H
#define RAFT_NODE_H

#include <memory>
#include <string>
#include <future>

#include <raftCore/raft_log.h>
#include <raftCore/kv_server.h>
#include <storage/persistent_storage.h>

#include "raft.pb.h"
#include "raft.grpc.pb.h" // 生成RaftRpcService::Stub
#include <grpcpp/grpcpp.h>

enum class Role {
    Follower, 
    Candidate, 
    Leader
};

struct Op {
    std::string Operation;  // 操作类型
    std::string Key;        // 键
    std::string Value;      // 值
    std::string ClientId;   // 客户端ID
    int RequestId;          // 请求ID

    Op() : Operation(""), Key(""), Value(""), ClientId(""), RequestId(0) {}
};

class RaftNode : public raft::RaftRPCService::Service {
public:
    // RaftNode(int id, const RaftConfig& config);

    RaftNode(int id, const std::vector<std::string>& peers, int me, PersistentStorage& storage)
        : role_(Role::Follower), currentTerm_(0), votedFor_(-1), log_(nullptr);

    ~RaftNode();

    // 禁止拷贝
    RaftNode(const RaftNode&) = delete;
    RaftNode& operator=(const RaftNode&) = delete;

    // 初始化 Raft 节点
    void init();

    // 将命令追加到 Leader 本地日志中，并持久化
    std::future<ApplyResult> Raft::start(Op command);

    // 重写基类方法
    // 领导者向其他节点发送日志复制和心跳消息
    void AppendEntries(google::protobuf::RpcController *controller, const ::raftRpcProctoc::AppendEntriesArgs *request,
        ::raftRpcProctoc::AppendEntriesReply *response, ::google::protobuf::Closure *done) override;

    // 请求投票
    void RequestVote(google::protobuf::RpcController *controller, const ::raftRpcProctoc::RequestVoteArgs *request,
        ::raftRpcProctoc::RequestVoteReply *response, ::google::protobuf::Closure *done) override;

    void applyCommittedLogs();              // 应用已提交日志到状态机
    void onElectionTimeout();               // 选举超时处理
    void onHeartbeatTimeout();              // 心跳超时处理
    void stepDown(int term);                // 降级为Follower

private:
    void becomeLeader();
    void becomeFollower(int term);
    void becomeCandidate();

    // 作为 Leader 发送心跳消息，包括发送日志，调整 index等
    void sendHeartbeats();

    // 心跳消息超时，触发选举，转变为 candidate，并向其他节点发起投票，不断尝试直到出现新的 leader
    void startElection();

    // 节点角色
    Role role_;
    int currentTerm_;
    int votedFor_;

    // 下一条即将发送的日志索引
    std::vector<int> next_index;

    // 已复制的最高日志索引
    std::vector<int> match_index;

    // 网络通信层，保存到其他节点的 RPC 连接
    std::vector<std::shared_ptr<RaftRPCC>> peers_;

    RaftLog* log_;
    PersistentStorage storage_;

    // 用定时器来处理选举和心跳，也可考虑协程或其他方法
    // Timer electionTimer_;
    // Timer heartbeatTimer_;
    // RaftRPC* rpc_;                   // 和其他节点进行网络通信
    // std::vector<std::shared_ptr<RaftRpcUtil>> m_peers;
};

#endif // RAFT_NODE_H