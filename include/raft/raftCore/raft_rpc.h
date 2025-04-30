#ifndef RAFT_RPC_H
#define RAFT_RPC_H

/*
    代表 Raft 节点之间的网络通信
*/
class RaftRPC {
public:
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
};
    
#endif // RAFT_RPC_H