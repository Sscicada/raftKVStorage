


## 节点通信


| 类型                    | 目的                      | 发起者               | 接收者         |
|-------------------------|---------------------------|----------------------|----------------|
| RequestVote RPC         | 选举时请求投票            | 候选人（Candidate）   | 其他所有节点    |
| AppendEntries RPC       | 复制日志 / 心跳保持       | Leader               | 其他所有节点    |
| InstallSnapshot RPC（可选） | 发送快照（日志过长时）     | Leader               | 落后节点        |
| 客户端请求转发           | Client请求键值操作时       | Follower → Leader    | Leader         |
| 集群配置变更（Membership Change） | 节点加入/离开时           | Leader               | 所有节点        |



```
raft/
├── raft_node.h / raft_node.cpp        # Raft 节点逻辑（选举、日志复制、状态转换）
├── log_entry.h / log_entry.cpp        # Raft 日志条目（封装 command + term + index）
├── raft_log.h / raft_log.cpp          # 日志存储与持久化管理（追加、截断、快照）
├── state_machine.h / state_machine.cpp# 状态机接口（上层 KV 应用实现）
├── timer.h / timer.cpp                # 选举/心跳超时定时器（精准控制选主）
├── raft_rpc.h / raft_rpc.cpp          # Raft 之间的 RPC 通信封装（RequestVote / AppendEntries）
├── raft_config.h / raft_config.cpp    # 配置管理（集群节点、端口、ID分配）
└── persistent_storage.h / cpp         # 持久化组件（保存term、vote、日志等）

```

| 模块 / 文件 | 是否论文中提到 | 功能说明 | 对应论文章节 |
|-------------|----------------|----------|----------------|
| `raft_node.h / cpp` | ✅  | Raft 核心状态机（领导选举、日志复制、状态转换） | 5.2 Leader Election、5.3 Log Replication |
| `log_entry.h / cpp` | ✅ | 日志条目（包含 `term`、`index`、command） | 5.3 Log Replication |
| `raft_log.h / cpp` | ✅ | 日志追加、截断、提交等 | 5.4 Log Commitment |
| `state_machine.h / cpp` | ✅ | 应用状态机（比如 KV 存储） | 7.1 Applying Logs |
| `timer.h / cpp` | ✅ | 实现选举超时、心跳时间间隔 | 9. Practical Considerations |
| `raft_rpc.h / cpp` | ✅ | 模拟论文中 RequestVote / AppendEntries 的 RPC 行为 | 5.2、5.3 |
| `raft_config.h / cpp` | ✅ | 每个节点 ID、集群配置 | 9.2 Membership Changes |
| `persistent_storage.h / cpp` | ✅ | term、votedFor、日志的持久化 | 7. Persistence |

### Raft 论文明确规定必须“持久化”的三样东西：
在论文 7.1 中列出，崩溃恢复时必须保存在磁盘上：

- 当前任期 (currentTerm)
- 已投给谁 (votedFor)
- 日志条目 (log[])

👉 这些都被放在 persistent_storage.cpp 里，符合规范。

```
raft/
├── core/              # Raft 核心状态机和日志
│   ├── raft_node.h / cpp         # 节点状态机（选举、复制、状态转换）
│   ├── raft_log.h / cpp          # 日志追加/回退/提交
│   ├── log_entry.h / cpp         # 单条日志定义
│   └── state_machine.h / cpp     # 应用状态机（如 KV 存储）
│
├── rpc/               # Raft 节点之间通信
│   ├── raft_rpc.h / cpp          # RPC 接口实现（调用封装）
│   ├── raft_rpc_server.h / cpp   # gRPC 服务端
│   ├── raft_rpc_client.h / cpp   # gRPC 客户端
│   └── raft.proto                # Protobuf 消息定义
│
├── timer/             # 定时器（选举、心跳）
│   ├── timer.h / cpp              # 通用定时器实现
│   └── election_timer.h / cpp    # 专用于选举的 Timer
│
├── storage/           # 持久化和快照
│   ├── persistent_storage.h / cpp  # 保存 term、vote、日志
│   └── snapshot.h / cpp            # 快照压缩机制（选做）
│
└── config/            # 配置和节点信息管理
    ├── raft_config.h / cpp         # 节点ID、端口、集群信息
    └── cluster_info.h / cpp        # 当前集群状态（成员、角色）
```

## Raft 的 init 函数：
启动 Raft 协议中的 3 个定时任务，分别是：

- 领导者发送心跳（leaderHearBeatTicker）

- 候选者发起选举（electionTimeOutTicker）

- 日志应用到状态机（applierTicker）

实现方式：
1、线程；2、定时器；3、协程；4、其他事件循环库