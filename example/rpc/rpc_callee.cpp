#include <vector>
#include <string>
#include <iostream>

#include <rpcchannel.h>
#include <rpcprovider.h>
#include <friend.pb.h>

const std::string ip = "127.0.0.1";
const short port = 4356;

class FriendService : public fixbug::FriendServiceRpc {
public:
    std::vector<std::string> GetFriendsList(uint32_t userid) {
        std::cout << "local do GetFriendsList service! userid:" << userid << std::endl;
        std::vector<std::string> vec;

        vec.push_back("xiao ming");
        vec.push_back("mei hong");
        vec.push_back("wang gang");

        return vec;
    }

    // 重写基类方法
    void GetFriendsList(::google::protobuf::RpcController *controller, const ::fixbug::GetFriendsListRequest *request,
                      ::fixbug::GetFriendsListResponse *response, ::google::protobuf::Closure *done) {
        uint32_t userid = request->userid();
        std::vector<std::string> friendsList = GetFriendsList(userid);

        response->mutable_result()->set_errcode(0);
        response->mutable_result()->set_errmsg("Get freind successfully!");

        for (std::string &name : friendsList) {
            std::string *p = response->add_friends();
            *p = name;
        }

        done->Run();
    }    
};

int main(int argc, char **argv) {
    auto stub = new fixbug::FriendServiceRpc_Stub(new RpcChannel(ip, port, false));
    // provider是一个rpc网络服务对象。把UserService对象发布到rpc节点上
    RpcProvider provider;
    provider.NotifyService(new FriendService());

    // 启动一个rpc服务发布节点   Run以后，进程进入阻塞状态，等待远程的rpc调用请求
    provider.Run(1, port);

    return 0;
}
