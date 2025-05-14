// #include "raft/raft_node.h"
// #include "kv/kv_state_machine.h"
// #include "rpc/rpc_server.h"
// #include "common/config.h"

#include <csignal>
#include <memory>
#include <iostream>

#include <rpcchannel.h>
#include <rpcprovider.h>
#include <raftClerk/clerk.h>
#include <raftCore/raft_log.h>
#include <raftCore/raft_node.h>
#include <raftCore/kv_server.h>
#include <storage/persistent_storage.h>

const std::string ip = "127.0.0.1";
const short port = 4357;

// 实现单节点同客户端通信，中间通过 kv 存储服务层
// 每个节点一个独立进程 + 本地 TCP 通信
int main() {
    auto stub = new kvServer::kvServerRpc_Stub(new RpcChannel(ip, port, false));
    // provider是一个rpc网络服务对象。把UserService对象发布到rpc节点上
    RpcProvider provider;
    // 服务注册
    provider.NotifyService(new KVServer());

    // 启动一个rpc服务发布节点   Run以后，进程进入阻塞状态，等待远程的rpc调用请求
    provider.Run(1, port);

    return 0;
}

// std::shared_ptr<RaftNode> g_raft_node;

// void signal_handler(int signal) {
//     if (g_raft_node) {
//         g_raft_node->shutdown();
//     }
//     exit(0);
// }

// int main(int argc, char** argv) {
//     if (argc != 2) {
//         printf("Usage: %s <config_path>\n", argv[0]);
//         return -1;
//     }

//     // 1. 加载配置
//     Config config(argv[1]);
//     int my_id = config.get_my_id();

//     // 2. 创建状态机
//     auto state_machine = std::make_shared<KVStateMachine>();

//     // 3. 创建 Raft Node
//     g_raft_node = std::make_shared<RaftNode>(my_id, config, state_machine);

//     // 4. 启动 Raft
//     g_raft_node->start();

//     // 5. 启动RPC Server，接收Raft RPC调用
//     RPCServer rpc_server(g_raft_node);
//     rpc_server.start(config.get_listen_port(my_id));

//     // 6. 安装信号处理（比如Ctrl+C）
//     signal(SIGINT, signal_handler);

//     // 7. 主线程监听客户端输入
//     std::string command;
//     while (std::getline(std::cin, command)) {
//         if (command.empty()) continue;
//         g_raft_node->submit_command(command);
//     }

//     return 0;
// }


// int main(int argc, char* argv[]) {
//     // 解析配置
//     int nodeId = std::stoi(argv[1]);
//     std::vector<Peer> peers = loadPeersFromConfig();
    
//     // 创建并启动Raft节点
//     RaftNode raftNode(nodeId, peers);
    
//     // 创建KV存储
//     KVStore kvStore;
    
//     // 启动RPC服务器
//     RaftRPCServer rpcServer;
//     rpcServer.RegisterRequestVoteHandler(
//         [&raftNode](const RequestVoteArgs& args, RequestVoteReply& reply) {
//             raftNode.HandleRequestVote(args, reply);
//         });
//     rpcServer.RegisterAppendEntriesHandler(
//         [&raftNode](const AppendEntriesArgs& args, AppendEntriesReply& reply) {
//             raftNode.HandleAppendEntries(args, reply);
//         });
//     rpcServer.Start();
    
//     // 主循环
//     while (true) {
//         // 应用已提交的日志到状态机
//         raftNode.ApplyLogsToStateMachine([&kvStore](const std::string& command) {
//             return kvStore.Apply(command);
//         });
        
//         // 其他维护工作...
//         std::this_thread::sleep_for(std::chrono::milliseconds(100));
//     }
    
//     rpcServer.Stop();
//     return 0;
// }