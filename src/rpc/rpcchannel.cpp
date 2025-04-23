#include <cerrno>
#include <string>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include "rpcchannel.h"
#include "rpccontroller.h"
#include "rpcheader.pb.h"
// #include "util.h"

/**
 * @brief 调用远程服务的方法，实现 RPC 请求和响应的统一处理。
 *
 * 该方法是 RPC 框架的核心，负责将请求序列化并通过网络发送到远程服务器，
 * 同时接收服务器的响应并反序列化为本地对象。
 * 简单来说，就是负责将服务的调用方的 request 请求数据发送给服务的提供方，
 * 并将服务提供方返回的 response 响应数据发给调用方。
 *
 * @param method [输入] 指向被调用的 RPC 方法的描述符。
 *               包含方法名、所属服务等信息。
 * @param controller [输入/输出] RPC 控制器，用于传递上下文信息（如超时、取消等）。
 *                   如果发生错误，可以通过 `controller->SetFailed()` 设置错误信息。
 * @param request [输入] 请求消息对象，表示要发送给服务器的参数。
 *               它是一个 Protobuf 定义的消息类型。
 * @param response [输出] 响应消息对象，用于存储从服务器接收到的响应数据。
 *                它也是一个 Protobuf 定义的消息类型。
 * @param done [输入] 回调函数指针，用于异步调用完成后的通知。
 *             如果是同步调用，可以传入 `nullptr`。
 *
 * @details
 * - 如果当前没有可用的 TCP 连接，会尝试重新连接。
 * - 请求数据格式为：header_size + rpc_header_str + args_str。
 *   - header_size: rpc_header_str 的长度（变长编码）。
 *   - rpc_header_str: 包含服务名、方法名和参数大小的序列化头部。
 *   - args_str: 序列化的请求参数。
 * - 响应数据直接是序列化的 Protobuf 消息。
 *
 * @note
 * - 如果发生错误，会通过 `controller->SetFailed()` 返回错误信息。
 * - 当前实现为短连接，未来可以改为长连接以提升性能。
 *
 * @see google::protobuf::RpcChannel
 */
void RpcChannel::CallMethod(const google::protobuf::MethodDescriptor* method,
                              google::protobuf::RpcController* controller, const google::protobuf::Message* request,
                              google::protobuf::Message* response, google::protobuf::Closure* done) {
    // 如果当前没有可用的 TCP 连接（client_fd_ == -1），尝试重新连接到服务器
    if (client_fd_ == -1) {
        std::string errMsg;
        bool rt = Connect(ip_.c_str(), port_, &errMsg);
        if (!rt) {
            printf("[func-MprpcChannel::CallMethod]重连接ip:{%s} port{%d}失败", ip_.c_str(), port_);
            controller->SetFailed(errMsg);
            return;
        }
        printf("[func-MprpcChannel::CallMethod]连接ip:{%s} port{%d}成功", ip_.c_str(), port_);
    }

    // 构造 RPC 请求头，首先获取服务名和服务方法
    const google::protobuf::ServiceDescriptor* sd = method->service();
    std::string service_name = sd->name();
    std::string method_name = method->name();
    std::cout << "channel service_name : " << service_name << '\n';
    std::cout << "channel method_name : " << method_name << '\n';

    // 获取参数的序列化字符串长度 args_size
    uint32_t args_size{};
    std::string args_str;

    // 将请求参数序列化为字符串
    if (request->SerializeToString(&args_str)) {
        args_size = args_str.size();
    } else {
        controller->SetFailed("serialize request error!");
        return;
    }

    // 构造 RPC 请求头部，设置服务名、方法名和参数大小
    RPC::RpcHeader rpcHeader;
    rpcHeader.set_service_name(service_name);
    rpcHeader.set_method_name(method_name);
    rpcHeader.set_args_size(args_size);

    std::string rpc_header_str;
    // 将 RpcHeader 序列化为字符串
    if (!rpcHeader.SerializeToString(&rpc_header_str)) {
        controller->SetFailed("serialize rpc header error!");
        return;
    }

    // 使用protobuf的CodedOutputStream来构建发送的数据流
    std::string send_rpc_str;  // 用来存储最终发送的数据
    {
        // 创建一个StringOutputStream用于写入send_rpc_str
        google::protobuf::io::StringOutputStream string_output(&send_rpc_str);
        google::protobuf::io::CodedOutputStream coded_output(&string_output);

        // 先写入header的长度（变长编码）
        coded_output.WriteVarint32(static_cast<uint32_t>(rpc_header_str.size()));

        // 不需要手动写入header_size，因为上面的WriteVarint32已经包含了header的长度信息
        // 然后写入rpc_header本身
        coded_output.WriteString(rpc_header_str);
    }

    // 最后，将请求参数附加到send_rpc_str后面
    send_rpc_str += args_str;

    // 打印调试信息
    //    std::cout << "============================================" << std::endl;
    //    std::cout << "header_size: " << header_size << std::endl;
    //    std::cout << "rpc_header_str: " << rpc_header_str << std::endl;
    //    std::cout << "service_name: " << service_name << std::endl;
    //    std::cout << "method_name: " << method_name << std::endl;
    //    std::cout << "args_str: " << args_str << std::endl;
    //    std::cout << "============================================" << std::endl;

    // 发送rpc请求
    // 失败会重试连接再发送，重试连接失败会直接return
    while (-1 == send(client_fd_, send_rpc_str.c_str(), send_rpc_str.size(), 0)) {
        char errtxt[512] = {0};
        sprintf(errtxt, "send error! errno:%d", errno);
        std::cout << "尝试重新连接，对方ip:" << ip_ << " 对方端口:" << port_ << std::endl;
        close(client_fd_);
        client_fd_ = -1;
        std::string errMsg;
        bool rt = Connect(ip_.c_str(), port_, &errMsg);
        if (!rt) {
          controller->SetFailed(errMsg);
          return;
        }
    }
    /*
        从时间节点来说，这里将请求发送过去之后，
        RPC 服务的提供者就会开始处理，返回的时候就代表着已经返回响应了
    */

    // 接收 RPC 请求的响应值
    // 缓冲区，用于接收响应数据
    char recv_buf[1024] = {0};
    int recv_size = 0;

    // 接收响应数据
    if (-1 == (recv_size = recv(client_fd_, recv_buf, 1024, 0))) {
        close(client_fd_);
        client_fd_ = -1;
        char errtxt[512] = {0};
        sprintf(errtxt, "recv error! errno:%d", errno);
        controller->SetFailed(errtxt);
        return;
    }

    // 反序列化 RPC 调用的响应数据
    // 注意：不能使用 ParseFromString，因为 recv_buf 中可能包含 \0 字符
    // std::string response_str(recv_buf, 0, recv_size); //
    // bug：出现问题，recv_buf中遇到\0后面的数据就存不下来了，导致反序列化失败 if
    // (!response->ParseFromString(response_str))
    if (!response->ParseFromArray(recv_buf, recv_size)) {
        char errtxt[1050] = {0};
        sprintf(errtxt, "parse error! response_str:%s", recv_buf);
        controller->SetFailed(errtxt);
        return;
    }
}

/**
 * @brief 创建一个新的 TCP 连接到指定的服务器。
 *
 * 该方法用于建立客户端与远程服务器之间的 TCP 连接。如果连接成功，则返回 true，
 * 并将连接的文件描述符存储在类成员变量 `client_fd_` 中；如果连接失败，则返回 false，
 * 并通过 `errMsg` 参数返回错误信息。
 *
 * @param ip [输入] 服务器的 IP 地址（字符串形式）。
 * @param port [输入] 服务器的端口号。
 * @param errMsg [输出] 如果连接失败，存储详细的错误信息。
 *
 * @details
 * - 使用 `socket()` 函数创建一个 TCP 套接字。
 * - 配置服务器地址（IP 和端口），并尝试通过 `connect()` 函数连接到服务器。
 * - 如果连接失败，会记录错误信息，并关闭已创建的套接字。
 * - 成功连接后，将套接字文件描述符存储在类成员变量 `client_fd_` 中。
 *
 * @note
 * - 如果 `socket()` 或 `connect()` 调用失败，错误信息会通过 `errMsg` 返回。
 * - 当前实现中，连接失败时会直接关闭套接字，确保资源释放。
 *
 * @see socket, connect
 */
 bool RpcChannel::Connect(const char* ip, uint16_t port, std::string* errMsg) {
    // 创建一个 TCP 套接字
    int clientfd = socket(AF_INET, SOCK_STREAM, 0);
    if (-1 == clientfd) {
        char errtxt[512] = {0};
        sprintf(errtxt, "Create socket error! errno:%d", errno); // 格式化错误信息
        client_fd_ = -1; // 确保类成员变量保持无效状态
        *errMsg = errtxt; // 将错误信息存储到 errMsg 中
        return false; // 返回连接失败
    }

    // 配置服务器地址
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET; // 设置地址族为 IPv4
    server_addr.sin_port = htons(port); // 设置端口号（转换为网络字节序）
    server_addr.sin_addr.s_addr = inet_addr(ip); // 设置服务器 IP 地址

    // 尝试连接到服务器
    if (-1 == connect(clientfd, (struct sockaddr*)&server_addr, sizeof(server_addr))) {
        close(clientfd); // 关闭已创建的套接字，避免资源泄露
        char errtxt[512] = {0};
        sprintf(errtxt, "Connect fail! errno:%d", errno); // 格式化错误信息
        client_fd_ = -1; // 确保类成员变量保持无效状态
        *errMsg = errtxt; // 将错误信息存储到 errMsg 中
        return false; // 返回连接失败
    }

    // 连接成功，更新类成员变量
    client_fd_ = clientfd;
    std::cout << "[DEBUG] Connecting to " << ip << ":" << port << " successfully!" << std::endl;
    return true; // 返回连接成功
}

/**
 * @brief 构造一个 MprpcChannel 对象，用于与指定的 RPC 服务器建立连接。
 *
 * 该构造函数初始化了客户端与 RPC 服务器的通信参数（IP 和端口），并根据 `connect` 参数决定是否立即尝试连接。
 * 如果 `connect` 为 true，则会尝试通过短连接方式与服务器建立连接，并支持多次重试机制。
 * 如果连接失败，错误信息会被打印到控制台。
 *
 * @param ip [输入] 服务器的 IP 地址（字符串形式）。
 * @param port [输入] 服务器的端口号。
 * @param connectNow [输入] 是否在构造对象时立即尝试连接服务器。
 *                  - 如果为 true，则会在构造函数中尝试连接。
 *                  - 如果为 false，则允许延迟连接。
 *
 * @details
 * - 使用 TCP 短连接方式完成 RPC 方法的远程调用。
 * - 在当前实现中，每次调用都需要重新连接服务器，未来可以改成长连接以提升性能。
 * - 如果 `connect` 为 true，则会调用 `newConnect` 方法尝试连接服务器。
 *   - 如果连接失败，会进行最多三次重试。
 *   - 每次重试都会打印错误信息到控制台。
 * - 成功连接后，套接字文件描述符会存储在类成员变量 `client_fd_` 中。
 *
 * @note
 * - 当前实现为短连接，每次调用都需要重新连接，可能会导致性能开销较大。
 * - 延迟连接功能可以通过设置 `connect` 为 false 来实现。
 * - 配置文件中的 `rpcserverip` 和 `rpcserverport` 可以动态加载，但此处直接使用构造函数传入的参数。
 *
 * @see newConnect, socket, connect
 */
 RpcChannel::RpcChannel(std::string ip, short port, bool connect) : ip_(ip), port_(port), client_fd_(-1) {
    // 使用 TCP 编程完成 RPC 方法的远程调用，当前实现为短连接，未来可改成长连接。
    // 如果没有连接或者连接已经断开，则需要重新连接，并支持不断重试。

    // 读取配置文件 rpcserver 的信息（示例代码）
    // std::string ip = MprpcApplication::GetInstance().GetConfig().Load("rpcserverip");
    // uint16_t port = atoi(MprpcApplication::GetInstance().GetConfig().Load("rpcserverport").c_str());

    // rpc 调用方想调用 service_name 的 method_name 服务，需要查询 zk 上该服务所在的 host 信息
    //  /UserServiceRpc/Login

    if (!connect) {
        return; // 如果 connectNow 为 false，允许延迟连接，不立即尝试连接
    }

    // 尝试连接服务器
    std::string errMsg;
    std::cout << "[DEBUG] Connecting to " << ip << ":" << port << "..." << std::endl;
    auto rt = Connect(ip.c_str(), port, &errMsg); // 调用 newConnect 方法尝试连接
    int tryCount = 1; // 最大重试次数
    while (!rt && tryCount--) {
        std::cout << errMsg << std::endl; // 打印错误信息
        rt = Connect(ip.c_str(), port, &errMsg); // 再次尝试连接
    }
    std::cout << "[DEBUG] Connecting failed " << std::endl;
}