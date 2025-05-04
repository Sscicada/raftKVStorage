#ifndef RAFT_STORAGE_PERSISTENT_STORAGE_H
#define RAFT_STORAGE_PERSISTENT_STORAGE_H

#include <vector>
#include <string>

#include "raftCore/raft_log.h"

/*
    Raft 协议自己需要的持久化存储组件。
    为了容错、重启恢复，需要把运行状态写入磁盘
*/
class PersistentStorage {
public:
    PersistentStorage() = default;

    void persist(int term, int votedFor, const std::vector<LogEntry>& log);
    void readPersist(int& term, int& votedFor, std::vector<LogEntry>& log) const;

private:
    std::string storagePath_;
};

#endif // RAFT_STORAGE_PERSISTENT_STORAGE_H