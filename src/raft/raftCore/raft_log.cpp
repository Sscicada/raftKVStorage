#include <raft_log.h>
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

void RaftLog::loadFromStorage(PersistentStorage& storage) {
    // 从持久化存储加载日志
    log_ = storage.loadLog();
    commitIndex_ = storage.loadCommitIndex();
    lastApplied_ = storage.loadLastApplied();
}

void RaftLog::saveToStorage(PersistentStorage& storage) {
    // 将日志保存到持久化存储
    storage.saveLog(log_);
    storage.saveCommitIndex(commitIndex_);
    storage.saveLastApplied(lastApplied_);
}

// 以上代码是 Raft 日志模块的实现，包含了日志的追加、截断、提交等操作，以及从持久化存储加载和保存日志的功能。
// 该模块是 Raft 协议的核心部分之一，负责管理和维护日志条目。
// 下面是一些可能的改进和扩展：
// 1. 日志压缩：
//    - 实现日志压缩算法（如快照）以减少存储空间。
// 2. 日志复制：
//    - 实现日志复制机制，以确保日志在集群中的一致性。
// 3. 错误处理：
//    - 增强错误处理机制，确保在存储或网络故障时能够正确恢复。
// 4. 性能优化：
//    - 优化日志存储和加载的性能，使用更高效的数据结构或算法。
// 5. 测试覆盖：
//    - 编写单元测试和集成测试，确保各个功能模块的正确性和稳定性。
// 6. 日志查询：    
//    - 实现日志查询接口，以便于调试和监控。
// 7. 日志格式化：
//    - 提供日志格式化工具，以便于人类可读性。
