#include <raftCore/raft_node.h>

void RaftNode::becomeLeader() {
    role_ = Role::Leader;
    currentTerm_++;
    votedFor_ = -1;  // 重置投票

    log_->persist();
    
    // 启动心跳和选举定时器
    startElection();
    sendHeartbeats();
}

void RaftNode::becomeFollower(int term) {
    role_ = Role::Follower;
    currentTerm_ = term;
    votedFor_ = -1;  // 重置投票

    log_->persist();
    
    // 重置选举定时器
    onElectionTimeout();
}

void RaftNode::becomeCandidate() {
    role_ = Role::Candidate;
    currentTerm_++;
    votedFor_ = -1;  // 重置投票

    log_->persist();
    
    // 启动选举
    startElection();
}

void RaftNode::start() {
    // 启动节点
    becomeFollower(0);
    // 启动定时器
    onElectionTimeout();
}

void RaftNode::applyCommittedLogs() {
    // 应用已提交日志到状态机
    auto logs = log_->getCommittedLogs();
    for (const auto& log : logs) {
        stateMachine_->apply(log);
    }
}

void RaftNode::onElectionTimeout() {
    // 选举超时处理
    if (role_ == Role::Leader) {
        return;
    }
    
    // 发起选举
    startElection();
}

void RaftNode::onHeartbeatTimeout() {
    // 心跳超时处理
    if (role_ == Role::Leader) {
        sendHeartbeats();
    }
}

void RaftNode::sendHeartbeats() {
    // 作为Leader发送心跳
    for (int i = 0; i < log_->getPeerCount(); ++i) {
        if (i != log_->getMyId()) {
            // 发送心跳请求
            log_->sendHeartbeat(i);
        }
    }
}

void RaftNode::startElection() {
    // 发起选举
    if (role_ == Role::Leader) {
        return;
    }
    
    becomeCandidate();
    
    // 发送投票请求
    for (int i = 0; i < log_->getPeerCount(); ++i) {
        if (i != log_->getMyId()) {
            log_->sendRequestVote(i);
        }
    }
}

void RaftNode::stepDown(int term) {
    // 降级为Follower
    if (term > currentTerm_) {
        becomeFollower(term);
    }
}