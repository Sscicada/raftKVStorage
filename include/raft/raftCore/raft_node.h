#ifndef RAFT_NODE_H
#define RAFT_NODE_H

#include "raft_log.h"
#include "state_machine.h"
#include "persistent_storage.h"

enum class Role {
    Follower, 
    Candidate, 
    Leader
};

class RaftNode {
public:
    RaftNode(int id, const RaftConfig& config);

    void start();                            // 启动节点
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

    void sendHeartbeats();                  // 作为Leader发送心跳
    void startElection();                   // 发起选举

    // 节点角色
    Role role_;
    int currentTerm_;
    int votedFor_;

    RaftLog log_;
    PersistentStorage storage_;
    KVStore* stateMachine_;            // KV应用

    // Timer electionTimer_;
    // Timer heartbeatTimer_;
    // RaftRPC* rpc_;                   // 和其他节点进行网络通信
};
    
#endif // RAFT_NODE_H