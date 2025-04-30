
### 启动集群中的一个节点

```shell
./kv_server \
  --id=1 \
  --port=5001 \
  --config=cluster.json \
  --data_dir=./data/node1 \
  --snapshot=true \
  --election_timeout=150
```

| 参数 | 含义 |
|------|------|
| `--id` | 节点 ID，需唯一 |
| `--port` | 本节点监听端口 |
| `--config` | 集群配置 JSON，含所有节点 IP+端口 |
| `--data_dir` | 持久化数据目录：日志、快照等 |
| `--snapshot` | 是否启用快照功能 |
| `--election_timeout` | 选举超时时间（ms） |

### 配置文件示例
```json
{
  "nodes": [
    { "id": 1, "ip": "127.0.0.1", "port": 5001 },
    { "id": 2, "ip": "127.0.0.1", "port": 5002 },
    { "id": 3, "ip": "127.0.0.1", "port": 5003 }
  ]
}

```
### 启动客户端
```shell
$ ./kv_cli --leader_ip 127.0.0.1 --leader_port 5001
> set name alice
OK
> get name
alice
> delete name
OK
```

### 启动可视化
```shell
./raft_gui --connect=127.0.0.1:5001
```