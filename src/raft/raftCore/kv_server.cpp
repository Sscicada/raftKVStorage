#include "state_machine.h"
#include <sstream>

void KVStore::apply(const LogEntry& entry) {
    std::istringstream iss(command);
    std::string op, key, value;
    iss >> op >> key;

    if (op == "set") {
        iss >> value;
        store_[key] = value;
    } else if (op == "delete") {
        store_.erase(key);
    }
}

std::string KVStore::query(const std::string& key) {
    auto it = store_.find(key);
    return it != store_.end() ? it->second : "(null)";
}