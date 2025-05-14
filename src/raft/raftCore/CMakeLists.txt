add_library(raftcore STATIC
        raft_log.cpp
        raft_node.cpp
        log_entry.cpp
        kv_server.cpp
 )