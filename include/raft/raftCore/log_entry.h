#ifndef RAFT_LOG_ENTRY_H
#define RAFT_LOG_ENTRY_H

#include <string>

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

    LogEntry() = default;
    LogEntry(int t, int i, const std::string& cmd)
        : term(t), index(i), command(cmd) {}
};

#endif // RAFT_LOG_ENTRY_H