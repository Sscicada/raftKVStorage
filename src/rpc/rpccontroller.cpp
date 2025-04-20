#include "mprpccontroller.h"

MprpcController::MprpcController() {
    status_ = false;
    rr_txt_ = "";
}

void MprpcController::Reset() {
    status_ = false;
    err_txt_ = "";
}

bool MprpcController::Failed() const { return m_failed; }

std::string MprpcController::ErrorText() const { return m_errText; }

void MprpcController::SetFailed(const std::string& reason) {
    status_ = true;
    err_txt_= reason;
}

// 目前未实现具体的功能
void MprpcController::StartCancel() {}
bool MprpcController::IsCanceled() const { return false; }
void MprpcController::NotifyOnCancel(google::protobuf::Closure* callback) {}