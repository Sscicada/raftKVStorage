#ifndef RPCPROVIDER_H
#define RPCPROVIDER_H

#include <string>
#include <unordered_map>

#include <muduo/net/EventLoop.h>
#include <muduo/net/InetAddress.h>
#include <muduo/net/TcpServer.h>
#include <muduo/net/TcpConnection.h>

/**
    provider类：专门发布 RPC 服务的网络对象类，提供了服务端注册和提供服务的接口。
    允许开发者将具体的业务逻辑绑定到某个服务接口上，并对外提供服务。
*/

class RpcProvider {
public:
    RpcProvider();
    ~RpcProvider();

    // 启动 RPC 服务节点，初始化 Muduo 的 TCP 服务器并绑定到指定端口
    // Muduo 的事件循环机制会自动监听客户端连接并分发事件
    void Run(int index, short port);

    // 用户可以将自定义的 RPC 服务注册到 RpcProvider 中
    // 服务的注册过程会解析服务的所有方法，并将其存储在 m_serviceMap 中
    void NotifyService(google::protobuf::Service* service);

private:
    // 服务信息
    struct ServiceInfo {
        google::protobuf::Service* service_;
        std::unordered_map<std::string, const google::protobuf::MethodDescriptor*> method_map_;
    };

    // 非阻塞事件循环，负责监听和调度网络事件
    muduo::net::EventLoop event_loop_;

    // 使用 std::shared_ptr 管理 Muduo TCP 服务器对象
    std::shared_ptr<muduo::net::TcpServer> muduo_server_;

    // 注册的服务信息
    std::unordered_map<std::string, ServiceInfo> service_map_;

    // 当客户端连接到服务器时，OnConnection 方法会被调用，记录连接状态
    void OnConnection(const muduo::net::TcpConnectionPtr &);

    // 当客户端发送请求时，OnMessage 方法会被触发，解析请求并调用对应的服务方法
    void OnMessage(const muduo::net::TcpConnectionPtr &, muduo::net::Buffer *, muduo::Timestamp);
  
    // Closure的回调操作，用于序列化rpc的响应和网络发送
    void SendRpcResponse(const muduo::net::TcpConnectionPtr &, google::protobuf::Message *);
};

#endif