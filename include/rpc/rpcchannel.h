/**
channel类：作为 RPC 通道，管理网络通信，封装底层的网络通信细节（如 TCP 或 UDP）
*/

#ifndef RPCCHANNEL_H
#define RPCCHANNEL_H

#include <iostream>
#include <string>
#include <unordered_map>

#include <google/protobuf/descriptor.h>
#include <google/protobuf/message.h>
#include <google/protobuf/service.h>

class RpcChannel : public google::protobuf::RpcChannel {
public:
    RpcChannel(std::string ip, short port, bool connect);

    void CallMethod(const google::protobuf::MethodDescriptor *method, google::protobuf::RpcController *controller,
        const google::protobuf::Message *request, google::protobuf::Message *response,
        google::protobuf::Closure *done) override;
private:
    std::string ip;
    uint16_t port;

    bool Connect();
};

#endif