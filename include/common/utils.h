
#include <chrono>
#include <random>

// 随机选举超时 (150-300ms)
std::chrono::milliseconds random_election_timeout() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(150, 300);
    return std::chrono::milliseconds(dis(gen));
}