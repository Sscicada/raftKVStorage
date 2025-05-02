#ifndef RAFT_CLERK_H
#define RAFT_CLERK_H

#include <vector>
#include <string>
#include <atomic>

class RaftClerk {
public:
    RaftClerk(int id, const std::vector<std::string>& servers);
    ~RaftClerk();

    void start();
    void stop();

    // 客户端请求接口
    bool put(const std::string& key, const std::string& value);
    std::string get(const std::string& key);
    bool del(const std::string& key);

    // 提交请求
    void submitRequest(const std::string& request);

    // 获取响应
    std::string getResponse();
private:
    int current_leader_ = -1;
    std::vector<std::string> cluster_addrs_;

    // 生成唯一的客户端ID
    std::string GenerateClientId() {
        static std::atomic<uint64_t> counter(0);
        return "client-" + std::to_string(counter++);
    }
};

#endif // RAFT_CLERK_H