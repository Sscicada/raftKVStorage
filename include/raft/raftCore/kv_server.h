#ifndef RAFT_STATE_MACHINE_H
#define RAFT_STATE_MACHINE_H

#include <string>
#include <unordered_map>

#include <raftCore/raft_log.h>

/*
    状态机：应用日志，即负责实际执行日志中的命令，更新系统状态
*/
class StateMachine {
public:
    virtual void apply(const LogEntry& entry) = 0;
    virtual std::string query(const std::string& key) = 0;
};

// 上层服务，负责 KV 存储
class KVServer : public StateMachine {
public:
    void apply(const LogEntry& entry) override;
    std::string query(const std::string& key) override;

private:
    // 暂时先用这个存储键值对
    // TODO：持久化
    std::unordered_map<std::string, std::string> store_;
};

#endif // RAFT_STATE_MACHINE_H