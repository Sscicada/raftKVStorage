#ifndef RAFT_STATE_MACHINE_H
#define RAFT_STATE_MACHINE_H

#include <string>
#include <vector>
#include <unordered_map>

#include <rpccontroller.h>
#include <raftCore/raft_log.h>
#include <server.pb.h>

// 上层服务，负责 KV 存储
class KVServer : public kvServer::kvServerRpc  {
public:
    KVServer();

    bool connect(std::vector<std::string> peers);

    // 处理客户端键值写入请求
    void PutAppend(::google::protobuf::RpcController* controller,
                   const ::kvServer::PutAppendArgs* request,
                   ::kvServer::PutAppendReply* response,
                   ::google::protobuf::Closure* done) override;

    // 处理客户端键值对获取请求
    void Get(::google::protobuf::RpcController* controller,
             const ::kvServer::GetArgs* request,
             ::kvServer::GetReply* response,
             ::google::protobuf::Closure* done) override;

    void apply(const LogEntry& entry);
    std::string query(const std::string& key);

private:
    // 暂时先用这个存储键值对
    // TODO：持久化
    std::unordered_map<std::string, std::string> store_;
};

#endif // RAFT_STATE_MACHINE_H