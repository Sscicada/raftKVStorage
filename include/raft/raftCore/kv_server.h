#ifndef RAFT_STATE_MACHINE_H
#define RAFT_STATE_MACHINE_H

#include <string>
#include <vector>
#include <unordered_map>

#include <raftCore/raft_log.h>

// 上层服务，负责 KV 存储
class KVServer {
public:
    bool connect(std::vector<std::string> peers);

    void apply(const LogEntry& entry);
    std::string query(const std::string& key);

private:
    // 暂时先用这个存储键值对
    // TODO：持久化
    std::unordered_map<std::string, std::string> store_;
};

#endif // RAFT_STATE_MACHINE_H