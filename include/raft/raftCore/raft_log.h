#ifndef RAFT_LOG_H
#define RAFT_LOG_H

#include <vector>
#include <optional>

#include <log_entry.h>
#include <persistent_storage.h>

/*
LogEntry 是日志复制和状态同步的最小单位。

在 Raft 中，每次客户端发来修改请求（如 set x 1），Leader 会：
1）把它封装成一条 LogEntry 日志；
2）通过 AppendEntries RPC 发送给 Follower；
3）多数节点复制后才能“提交”；
4）然后应用到状态机中。
*/
struct LogEntry {
    int term;                   // 写入该条日志时，Leader 的当前任期（currentTerm）
    int index;                  // 日志序列中的全局递增编号，每个日志项的 index 是唯一的
    std::string command;        // 序列化后的客户端命令

    LogEntry(int t, int i, const std::string& cmd)
        : term(t), index(i), command(cmd) {}
};

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