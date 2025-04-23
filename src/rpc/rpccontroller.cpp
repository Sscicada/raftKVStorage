#include "rpccontroller.h"

RpcController::RpcController() {
    status_ = false;
    // rr_txt_ = "";
}

void RpcController::Reset() {
    status_ = false;
    err_txt_ = "";
}

bool RpcController::Failed() const { return status_; }

std::string RpcController::ErrorText() const { return err_txt_; }

void RpcController::SetFailed(const std::string& reason) {
    status_ = true;
    err_txt_= reason;
}

// 目前未实现具体的功能
void RpcController::StartCancel() {}
bool RpcController::IsCanceled() const { return false; }
void RpcController::NotifyOnCancel(google::protobuf::Closure* callback) {}