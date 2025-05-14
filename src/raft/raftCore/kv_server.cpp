#include <sstream>

#include <raftCore/kv_server.h>

// 构造函数，初始化 kv 存储服务以及 raft 集群
KVServer::KVServer() {

}

// 处理客户端键值写入请求
void KVServer::PutAppend(::google::protobuf::RpcController* controller,
               const ::kvServer::PutAppendArgs* request,
               ::kvServer::PutAppendReply* response,
               ::google::protobuf::Closure* done) {

}

// 处理客户端键值对获取请求
void KVServer::Get(::google::protobuf::RpcController* controller,
         const ::kvServer::GetArgs* request,
         ::kvServer::GetReply* response,
         ::google::protobuf::Closure* done) {

}

void KVServer::apply(const LogEntry& entry) {
    // std::istringstream iss(command);
//    std::string op, key, value;
//    iss >> op >> key;
//
//    if (op == "set") {
//        iss >> value;
//        store_[key] = value;
//    } else if (op == "delete") {
//        store_.erase(key);
//    }
}

std::string KVServer::query(const std::string& key) {
    auto it = store_.find(key);
    return it != store_.end() ? it->second : "(null)";
}