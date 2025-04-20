
#ifndef RPCCONTROLLER_H
#define RPCCONTROLLER_H

#include <string>

#include <google/protobuf/service.h>

/**
    controller类：控制器类，控制 RPC 调用的行为，
    例如设置超时时间、取消请求等，也可以获取 RPC 调用的状态
*/

class RpcController : public google::protobuf::RpcController {
public:
    RpcController();

    void Reset();
    void SetFailed(const std::string& reason);

    void StartCancel();
    bool IsCanceled() const;
    void NotifyOnCancel(google::protobuf::Closure* callback);

    bool GetStatus() const;
    std::string GetErrorText() const;
private:
    // RPC 执行状态
    bool status_;
    // 错误消息
    std::string err_txt_;
};

#endif