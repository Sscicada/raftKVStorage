#ifndef KV_SERVER_H
#define KV_SERVER_H

#include <string>
#include <vector>

class KVServer {
public:
    KVServer(int id, const std::vector<std::string>& servers);
    ~KVServer();

    void start();
    void stop();

    // 客户端请求接口
    bool put(const std::string& key, const std::string& value);
    std::string get(const std::string& key);
    bool del(const std::string& key);
};

#endif // KV_SERVER_H