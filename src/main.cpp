// #include "raft/raft_node.h"
// #include "kv/kv_state_machine.h"
// #include "rpc/rpc_server.h"
// #include "common/config.h"

#include <csignal>
#include <memory>
#include <iostream>

#include <raftCore/raft_log.h>
#include <raftCore/state_machine.h>
#include <storage/persistent_storage.h>

/*
单节点追加日志
*/
int main() {
    // 初始化日志模块
    RaftLog raftLog(logEntries);

    // 初始化状态机（KV存储）
    KVStore stateMachine;

    // 构造并追加一个新日志条目
    LogEntry entry;
    entry.term = currentTerm;
    entry.index = raftLog.nextIndex();  // 假设 nextIndex() 给出当前应写入的位置
    entry.command = "set x 1";

    raftLog.appendEntry(entry);
    // storage.saveLog(raftLog.getEntries()); // 持久化日志

    // 应用日志到状态机（模拟 commit）
    const auto& committedLogs = raftLog.getEntries();
    for (const auto& e : committedLogs) {
        stateMachine.apply(e);
    }

    // 查询状态机是否成功写入
    std::cout << "Query result of key 'x': " << stateMachine.query("x") << std::endl;

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