#include <iostream>

#include <friend.pb.h>

#include <rpcchannel.h>
#include <rpccontroller.h>
#include <rpcprovider.h>

const std::string ip = "172.17.55.111";
const short port = 4356;

int main(int argc, char **argv) {
    // https://askubuntu.com/questions/754213/what-is-difference-between-localhost-address-127-0-0-1-and-127-0-1-1
    // 演示调用远程发布的rpc方法
    //注册进自己写的channel类，channel类用于自定义发送格式和负责序列化等操作
    RpcChannel channel(ip, port, true);
    fixbug::FriendServiceRpc_Stub stub(&channel);

    // 构建 RPC 方法的请求参数
    fixbug::GetFriendsListRequest request;
    request.set_userid(1000);

    // RPC 的响应
    fixbug::GetFriendsListResponse response;
    // 发起rpc方法的调用,消费这的stub最后都会调用到channel的 call_method方法  同步的rpc调用过程  rpcChannel::callmethod
    RpcController controller;
    // 10 次 RPC 方法调用
    int count = 10;
    while (count--) {
        std::cout << " 倒数第" << count << "次发起RPC请求" << std::endl;
        stub.GetFriendsList(&controller, &request, &response, nullptr);
        // RpcChannel->RpcChannel::callMethod 集中来做所有rpc方法调用的参数序列化和网络发送

        // RPC 调用失败
        if (controller.Failed()) {
            std::cout << "RPC failed reason :" << controller.ErrorText() << std::endl;
        } else {
            if (0 == response.result().errcode()) {
                std::cout << "rpc GetFriendsList response success!" << std::endl;
                int size = response.friends_size();
                for (int i = 0; i < size; i++) {
                    std::cout << "index:" << (i + 1) << " name:" << response.friends(i) << std::endl;
                }
            } else {
                // 业务逻辑失败
                std::cout << "rpc GetFriendsList response error : " << response.result().errmsg() << std::endl;
            }
        }
        sleep(5);  // sleep 5 seconds
    }
  return 0;
}
