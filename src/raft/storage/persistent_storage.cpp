#include "persistent_storage.h"
#include <nlohmann/json.hpp>
#include <fstream>

void PersistentStorage::persist(int term, int votedFor, const std::vector<LogEntry>& log) {

}

void PersistentStorage::readPersist(int& term, int& votedFor, std::vector<LogEntry>& log) const {

}