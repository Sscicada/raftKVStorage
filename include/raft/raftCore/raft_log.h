#ifndef RAFT_LOG_H
#define RAFT_LOG_H

#include <vector>
#include <string>
#include <optional>

#include "raftCore/log_entry.h"
#include "storage/persistent_storage.h"

/*
    Raft 日志存储组件，负责持久化存储和日志复制
    1. 追加日志
    2. 截断日志
    3. 提交日志
    4. 获取日志
    5. 从持久化存储加载日志
    6. 保存日志到持久化存储
*/
class RaftLog {
public:
    void appendEntry(const LogEntry& entry);    // 追加日志
    void truncateFrom(int index);               // 截断日志
    void commitTo(int index);                   // 提交日志
    std::optional<LogEntry> getEntry(int index) const;

    int lastLogIndex() const;               // 获取最后一条日志的索引
    int lastLogTerm() const;                // 获取最后一条日志的任期
    int commitIndex() const;                // 获取提交索引
    int lastApplied() const;                // 获取已应用索引

    void loadFromStorage(PersistentStorage& storage);
    void saveToStorage(PersistentStorage& storage);

private:
    std::vector<LogEntry> log_;
    int commitIndex_;
    int lastApplied_;
};

#endif // RAFT_LOG_H