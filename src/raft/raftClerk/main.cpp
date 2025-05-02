#include <iostream>
#include <string>

#include <raftClerk/clerk.h>

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <cluster_addresses>" << std::endl;
        return 1;
    }

    try {
        // std::vector<std::string> cluster{argv + 1, argv + argc};
        // RaftClerk client;

        // TODO：客户端随机连接节点，由 Follower 转发请求到 Leader
        std::cout << "Connected to cluster. Type 'exit' to quit." << std::endl;
        
        while (true) {
            std::string input;
            std::cout << "> ";
            std::getline(std::cin, input);
            
            if (input == "exit") break;
            
            // TODO: 实现完整的命令行解析
            if (input.substr(0, 3) == "get") {
                std::string key = input.substr(4);
                std::cout << "get " << key << std::endl;
                // std::cout << client.get(key) << std::endl;
            } else if (input.substr(0, 3) == "put") {
                size_t space = input.find(' ', 4);
                if (space != std::string::npos) {
                    std::string key = input.substr(4, space - 4);
                    std::string value = input.substr(space + 1);
                    // client.put(key, value);
                    std::cout << "OK" << std::endl;
                }
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}