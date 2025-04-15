
#ifndef RPCPROVIDER_H
#define RPCPROVIDER_H

#include <string>
#include <unordered_map>

#include <muduo/net/EventLoop.h>
#include <muduo/net/InetAddress.h>
#include <muduo/net/TcpServer.h>
#include <muduo/net/TcpConnection.h>

/**
provider类：提供了服务端注册和提供服务的接口。允许开发者将具体的业务逻辑绑定到某个服务接口上，并对外提供服务。
*/

class RpcProvider {
public:
    // 启动 RPC 服务
    void Run(int index, short port);

    // 发布 RPC 方法
    void NotifyService(google::protobuf::Service* service);

private:
    // 服务信息
    struct ServiceInfo {
        google::protobuf::Service* service_;
        std::unordered_map<std::string, const google::protobuf::MethodDescriptor*> method_map_;
    };

    // 注册的服务信息
    std::unordered_map<std::string, ServiceInfo> service_map_;
};

#endif