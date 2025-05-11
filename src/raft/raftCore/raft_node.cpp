#include <raftCore/raft_node.h>

void RaftNode::init(const std::vector<std::string>& peers, int me, RaftLog* log, PersistentStorage& storage) {
    // 加载持久化数据
    // readPersisit();

    role_ = Role::Follower;


    // TODO：待优化，换成协程或定时器任务
    // 启动三个线程
//    std::thread doHeartbeatThread(&RaftNode::sendHeartBeatTicker, this);
//    doHeartbeatThread.detach();

    std::thread doElectionThread(&RaftNode::startElection, this);
    doElectionThread.detach();

    std::thread applyLogsThread(&RaftNode::applyCommittedLogs, this);
    applyLogsThread.detach();
}

void RaftNode::becomeLeader() {
    role_ = Role::Leader;
    term_++;
    voted_for_ = -1;  // 重置投票
    
    // 启动心跳和选举定时器
}

void RaftNode::becomeFollower(int term) {
    role_ = Role::Follower;
    term_ = term;
    voted_for_ = -1;  // 重置投票
    
    // 重置选举定时器
    onElectionTimeout();
}

void RaftNode::becomeCandidate() {
    role_ = Role::Candidate;
    term_++;
    voted_for_ = -1;  // 重置投票
    
    // 启动选举
    startElection();
}

void RaftNode::start(Op command) {
    // 启动节点
    becomeFollower(0);
    // 启动定时器
    onElectionTimeout();
}

void RaftNode::startElection() {
    // 发起选举
    if (role_ == Role::Leader) {
        return;
    }
    
    becomeCandidate();
    
    // 发送投票请求
}

void RaftNode::applyCommittedLogs() {
}

void RaftNode::onElectionTimeout() {
    // 选举超时处理
    if (role_ == Role::Leader) {
        return;
    }
    
    // 发起选举
    startElection();
}

void RaftNode::stepDown(int term) {
    // 降级为Follower
    if (term > term_) {
        becomeFollower(term);
    }
}

void RaftNode::AppendEntries(::google::protobuf::RpcController *controller, const ::raft::AppendEntriesRequest *request,
                   ::raft::AppendEntriesReply *response, ::google::protobuf::Closure *done) {

}

void RaftNode::RequestVote(::google::protobuf::RpcController *controller, const ::raft::RequestVoteRequest *request,
                 ::raft::RequestVoteReply *response, ::google::protobuf::Closure *done) {

}