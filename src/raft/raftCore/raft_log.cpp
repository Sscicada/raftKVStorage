#include <raftCore/raft_log.h>
#include <stdexcept>

void RaftLog::appendEntry(const LogEntry& entry) {
    log_.push_back(entry);
}

void RaftLog::truncateFrom(int index) {
    if (index < 0 || index >= log_.size()) {
        throw std::out_of_range("Index out of range");
    }
    log_.erase(log_.begin() + index, log_.end());
}

void RaftLog::commitTo(int index) {
    if (index < 0 || index >= log_.size()) {
        throw std::out_of_range("Index out of range");
    }
    commitIndex_ = index;
}

std::optional<LogEntry> RaftLog::getEntry(int index) const {
    if (index < 0 || index >= log_.size()) {
        return std::nullopt;
    }
    return log_[index];
}

int RaftLog::lastLogIndex() const {
    return static_cast<int>(log_.size()) - 1;
}

int RaftLog::lastLogTerm() const {
    if (log_.empty()) {
        return -1; // 或者其他适当的值
    }
    return log_.back().term;
}

int RaftLog::commitIndex() const {
    return commitIndex_;
}

int RaftLog::lastApplied() const {
    return lastApplied_;
}

// void RaftLog::loadFromStorage(PersistentStorage& storage) {
//     // 从持久化存储加载日志
//     log_ = storage.readPersist();
//     commitIndex_ = storage.loadCommitIndex();
//     lastApplied_ = storage.loadLastApplied();
// }

// void RaftLog::saveToStorage(PersistentStorage& storage) {
//     // 将日志保存到持久化存储
//     storage.saveLog(log_);
//     storage.saveCommitIndex(commitIndex_);
//     storage.saveLastApplied(lastApplied_);
// }
