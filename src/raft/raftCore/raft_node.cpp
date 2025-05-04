#include <raftCore/raft_node.h>

void RaftNode::init() {
    // 加载持久化数据
    // readPersisit();

    role_ = Role::Follower;


    // TODO：待优化，换成协程或定时器任务
    // 启动三个线程
    std::thread doHeartbeatThread(&RaftNode::sendHeartBeatTicker, this);
    doHeartbeatThread.detach();

    std::thread doElectionThread(&RaftNode::startElection, this);
    doElectionThread.detach();

    std::thread applyLogsThread(&RaftNode::applyCommittedLogs, this);
    applyLogsThread.detach();
}

void RaftNode::becomeLeader() {
    role_ = Role::Leader;
    term_++;
    voted_for_ = -1;  // 重置投票

    log_->persist();
    
    // 启动心跳和选举定时器
    startElection();
    sendHeartbeats();
}

void RaftNode::becomeFollower(int term) {
    role_ = Role::Follower;
    term_ = term;
    voted_for_ = -1;  // 重置投票

    log_->persist();
    
    // 重置选举定时器
    onElectionTimeout();
}

void RaftNode::becomeCandidate() {
    role_ = Role::Candidate;
    term_++;
    voted_for_ = -1;  // 重置投票

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

void RaftNode::sendHeartBeatTicker() {
    while (true) {
        {
            std::unique_lock<std::mutex> lock(m_mtx);
            if (m_status != Leader) {
                DPrintf("Node %d is no longer leader. Exit heartbeat ticker.\n", m_me);
                return;
            }
        }

        doHeartBeat();  // 发送心跳 + 日志同步

        std::this_thread::sleep_for(std::chrono::milliseconds(heartbeatIntervalMs));
    }
}

bool RaftNode::sendRequestVote(int index, std::shared_ptr<raftRpcProctoc::RequestVoteRequest> request,
    std::shared_ptr<raftRpcProctoc::RequestVoteReply> reply, std::shared_ptr<int> votedNum) {
    // 发送投票请求
    bool ok = peers_[index]->RequestVote(request.get(), reply.get());
    if (!ok) {
        return false;
    }

    if (reply->term() > this->term_) {
        // 收到更高任期的请求，降级为 Follower
        stepDown(reply->term());
        return false;
    }

    if (reply->term() < this->term_) {
        // 收到过期的请求，忽略
        return false;
    }

    if (reply->vote_granted()) {
        // 收到投票，增加投票数
        (*votedNum)++;
        if (*votedNum > peers_.size() / 2) {
            // 获得大多数投票，成为 Leader
            becomeLeader();
        }
    }
    return true;
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

void RaftNode::stepDown(int term) {
    // 降级为Follower
    if (term > currentTerm_) {
        becomeFollower(term);
    }
}