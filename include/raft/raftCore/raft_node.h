#ifndef RAFT_NODE_H
#define RAFT_NODE_H

#include "raft_log.h"
#include "state_machine.h"
#include "storage/persistent_storage.h"

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

class RaftNode {
public:
    // RaftNode(int id, const RaftConfig& config);

    // 禁止拷贝
    RaftNode(const RaftNode&) = delete;
    RaftNode& operator=(const RaftNode&) = delete;

    // 将命令追加到 Leader 本地日志中，并持久化
    void start(Op commands);

    // void handleAppendEntries(const AppendEntriesRequest& req); // 处理追加日志请求
    // void handleRequestVote(const RequestVoteRequest& req);      // 处理投票请求
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

    RaftLog* log_;
    PersistentStorage storage_;
    KVStore* stateMachine_;            // KV应用

    // Timer electionTimer_;
    // Timer heartbeatTimer_;
    // RaftRPC* rpc_;                   // 和其他节点进行网络通信
    // std::vector<std::shared_ptr<RaftRpcUtil>> m_peers;
};

#endif // RAFT_NODE_H