#include "rpcprovider.h"
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <cstring>
#include <fstream>
#include <string>
#include "rpcheader.pb.h"
#include "util.h"

/**
 * @brief 注册服务到 RPC 框架中。
 *
 * 该方法用于将一个 Protobuf 定义的服务注册到框架的内部服务映射表中，
 * 以便后续可以通过服务名和方法名快速定位到具体的 RPC 方法。
 *
 * @param service [输入] 指向要注册的 Protobuf 服务对象。
 *                该对象包含了服务的所有方法以及相关的描述信息。
 *
 * @details
 * - 获取服务的描述符（ServiceDescriptor），从中提取服务名称和服务定义的方法列表。
 * - 遍历服务的所有方法，将其方法名与对应的描述符存储到 `method_map_` 中。
 * - 将服务信息（包括服务对象和方法映射）存储到全局的服务映射表 `service_map_` 中。
 * - 输出日志以确认服务已成功注册。
 *
 * @note
 * - 每个服务的名称是唯一的，重复注册可能会覆盖之前的服务信息。
 * - 方法映射表（method_map_）通过方法名作为键值，确保快速查找。
 *
 * @see google::protobuf::Service
 * @see google::protobuf::ServiceDescriptor
 * @see google::protobuf::MethodDescriptor
 */
void RpcProvider::NotifyService(google::protobuf::Service *service) {
    // 注册服务，首先需要创建一个 ServiceInfo
    ServiceInfo service_info;

    // 获取了服务对象的描述信息和服务名
    const google::protobuf::ServiceDescriptor *serviceDesc = service->GetDescriptor();
    std::string service_name = serviceDesc->name();

    std::cout << "service_name :" << service_name << std::endl;

    // 遍历服务中的所有方法，获取方法描述和方法名并存入 ServiceInfo 的 method_map_
    for (int i = 0; i < serviceDesc->method_count(); ++i) {
      const google::protobuf::MethodDescriptor *methodDesc = serviceDesc->method(i);
      std::string method_name = methodDesc->name();
      service_info.method_map_[method_name] = methodDesc;
    }

    service_info.service_ = service;
    service_map_[service_name] = service_info;
    std::cout << "Registered service :" << serviceName << std::endl;
}

/**
 * @brief 启动 RPC 服务并绑定到指定的 IP 和端口。
 *
 * 该方法用于初始化和启动 RPC 提供者的服务端，完成以下任务：
 * - 获取本机可用的 IP 地址。
 * - 将服务节点信息写入配置文件。
 * - 创建 TCP 服务器并绑定连接和消息回调函数。
 * - 设置线程数量并启动事件循环以处理网络请求。
 *
 * @param nodeIndex [输入] 当前服务节点的索引号，用于区分多个服务节点。
 * @param port [输入] 指定的服务端监听端口号。
 *
 * @details
 * - **获取本地 IP 地址**：
 *   使用 `gethostname` 和 `gethostbyname` 获取本机主机名及其对应的 IP 地址列表。
 *   遍历地址列表，提取第一个有效的 IPv4 地址作为服务绑定的 IP。
 * 
 * - **写入配置文件**：
 *   将服务节点的 IP 和端口信息追加写入到 `test.conf` 文件中，格式为：
 *   ```
 *   node<nodeIndex>ip = <IP>
 *   node<nodeIndex>port = <Port>
 *   ```
 *   如果文件打开失败，则输出错误日志并终止程序。
 *
 * - **创建 TCP 服务器**：
 *   使用 Muduo 网络库创建一个 TCP 服务器对象，并绑定到指定的 IP 和端口。
 *
 * - **绑定回调函数**：
 *   - `OnConnection`：处理连接建立和断开事件。
 *   - `OnMessage`：处理接收到的消息数据。
 *   使用 `std::bind` 将回调函数与当前对象（`this`）绑定，确保回调函数能够访问对象的状态。
 *
 * - **设置线程池**：
 *   设置 Muduo 库的工作线程数量（默认为 4），以支持多线程并发处理网络事件。
 *
 * - **启动服务**：
 *   调用 `muduo_server->start()` 启动服务器，并进入事件循环（`eventLoop.loop()`）以持续监听和处理网络事件。
 *
 * @note
 * - 配置文件 `test.conf` 的写入操作使用了追加模式（`std::ios::app`），避免覆盖已有内容。
 * - 事件循环会阻塞主线程，直到服务被显式停止。
 *
 * @see muduo::net::TcpServer
 * @see muduo::net::EventLoop
 * @see std::bind
 */
void RpcProvider::Run(int nodeIndex, short port) {
    //获取可用ip
    char *ipC;
    char hname[128];
    struct hostent *hent;
    gethostname(hname, sizeof(hname));
    hent = gethostbyname(hname);
    for (int i = 0; hent->h_addr_list[i]; i++) {
      ipC = inet_ntoa(*(struct in_addr *)(hent->h_addr_list[i]));  // IP地址
    }

    std::string ip = std::string(ipC);
    //    // 获取端口
    //    if(getReleasePort(port)) //在port的基础上获取一个可用的port，不知道为何没有效果
    //    {
    //        std::cout << "可用的端口号为：" << port << std::endl;
    //    }
    //    else
    //    {
    //        std::cout << "获取可用端口号失败！" << std::endl;
    //    }

    //写入文件 "test.conf"
    std::string node = "node" + std::to_string(nodeIndex);
    std::ofstream outfile;
    outfile.open("test.conf", std::ios::app);

    if (!outfile.is_open()) {
      std::cout << "打开文件失败！" << std::endl;
      exit(EXIT_FAILURE);
    }

    outfile << node + "ip = " + ip << std::endl;
    outfile << node + "port = " + std::to_string(port) << std::endl;
    outfile.close();

    //创建服务器
    muduo::net::InetAddress address(ip, port);

    // 创建TcpServer对象
    muduo_server = std::make_shared<muduo::net::TcpServer>(&eventLoop, address, "RpcProvider");

    // 绑定连接回调和消息读写回调方法  分离了网络代码和业务代码
    /*
    bind的作用：
    如果不使用std::bind将回调函数和TcpConnection对象绑定起来，那么在回调函数中就无法直接访问和修改TcpConnection对象的状态。因为回调函数是作为一个独立的函数被调用的，它没有当前对象的上下文信息（即this指针），也就无法直接访问当前对象的状态。
    如果要在回调函数中访问和修改TcpConnection对象的状态，需要通过参数的形式将当前对象的指针传递进去，并且保证回调函数在当前对象的上下文环境中被调用。这种方式比较复杂，容易出错，也不便于代码的编写和维护。因此，使用std::bind将回调函数和TcpConnection对象绑定起来，可以更加方便、直观地访问和修改对象的状态，同时也可以避免一些常见的错误。
    */
    muduo_server->setConnectionCallback(std::bind(&RpcProvider::OnConnection, this, std::placeholders::_1));
    muduo_server->setMessageCallback(
        std::bind(&RpcProvider::OnMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

    // 设置 muduo 库的线程数量
    muduo_server->setThreadNum(4);

    // 准备启动 RPC 服务，打印信息
    std::cout << "RpcProvider start service at ip:" << ip << " port:" << port << std::endl;

    // 启动网络服务，然后进入事件循环阶段，等待并处理各种事件
    muduo_server->start();
    std::cout << "RpcProvider is running on port " << port << " with index " << index << std::endl;
    eventLoop.loop();
}

/**
 * @brief 回调函数，处理 TCP 连接的建立和关闭事件。
 *
 * 该方法是连接事件的回调函数，用于在客户端与服务器之间建立或断开连接时执行相应的操作。
 * 它会根据连接状态输出日志信息，方便调试和监控。
 *
 * @param conn [输入] 指向当前 TCP 连接的智能指针。
 *             包含了连接的相关信息，例如对端地址、连接状态等。
 *
 * @details
 * - 如果连接已建立（`conn->connected()` 返回 true），输出新连接的对端 IP 和端口号。
 * - 如果连接已关闭（`conn->connected()` 返回 false），输出连接关闭的对端 IP 和端口号。
 * - 使用 `conn->peerAddress().toIpPort()` 获取对端地址的字符串表示形式。
 *
 * @note
 * - 该方法通常由网络库（如 Muduo）在连接状态变化时自动调用。
 * - 输出的日志信息可用于跟踪客户端的连接行为以及排查网络问题。
 *
 * @see muduo::net::TcpConnection
 */
void RpcProvider::OnConnection(const muduo::net::TcpConnectionPtr &conn) {
    // 新连接
    if (conn->connected()) {
        std::cout << "New connection from " << conn->peerAddress().toIpPort() << std::endl;
    } else {
        // 连接断开
        conn->shutdown();
        std::cout << "Connection closed from " << conn->peerAddress().toIpPort() << std::endl;
    }
}

/**
 * @brief 处理远程 RPC 调用请求的回调函数。
 *
 * 该方法是 RPC 框架的核心逻辑之一，用于解析客户端发送的 RPC 请求数据，
 * 并根据服务名、方法名和参数调用本地注册的服务方法。
 * 它实现了从网络数据到本地业务逻辑的映射，并通过 Protobuf 进行序列化和反序列化操作。
 *
 * @param conn [输入] 指向当前 TCP 连接的智能指针。
 *             包含了连接的相关信息，例如对端地址、连接状态等。
 * @param buffer [输入] 指向接收数据缓冲区的指针。
 *              包含了客户端发送的原始数据流。
 * @param timestamp [输入] 数据到达的时间戳。
 *
 * @details
 * - **解析数据头**：
 *   使用 Protobuf 的 `CodedInputStream` 解析接收到的数据流。
 *   数据格式为：`header_size(4字节) + header_str + args_str`。
 *   - `header_size`：表示数据头的长度（变长编码）。
 *   - `header_str`：包含服务名、方法名和参数大小的序列化头部。
 *   - `args_str`：序列化的请求参数。
 *
 * - **反序列化数据头**：
 *   根据 `header_size` 提取数据头字符串并反序列化为 `RpcHeader` 对象。
 *   从中提取服务名、方法名和参数大小。
 *
 * - **提取请求参数**：
 *   根据参数大小读取请求参数的字符串数据。
 *
 * - **查找服务和方法**：
 *   在服务映射表中查找对应的服务对象和方法描述符。
 *   如果服务或方法不存在，则输出错误日志并返回。
 *
 * - **生成请求和响应对象**：
 *   根据方法描述符创建请求和响应对象，并将请求参数反序列化为请求对象。
 *
 * - **绑定回调函数**：
 *   为 RPC 方法调用绑定一个回调函数（Closure），在方法执行完成后序列化响应并通过网络发送回客户端。
 *
 * - **调用本地业务方法**：
 *   使用 `service->CallMethod()` 调用本地注册的服务方法。
 *   - `CallMethod` 是 Protobuf 自动生成的代码中的核心方法。
 *   - 它会根据传入的方法描述符自动调用用户实现的具体业务逻辑。
 *
 * @note
 * - 数据格式的设计确保了请求的可扩展性和兼容性。
 * - 使用 Protobuf 的序列化和反序列化机制保证了数据传输的可靠性和效率。
 * - 回调函数的设计使得异步调用成为可能，提升了框架的灵活性。
 *
 * @see google::protobuf::Service
 * @see google::protobuf::MethodDescriptor
 * @see google::protobuf::Message
 * @see muduo::net::TcpConnection
 */
void RpcProvider::OnMessage(const muduo::net::TcpConnectionPtr &conn, muduo::net::Buffer *buffer, muduo::Timestamp) {
    // 网络上接收的远程rpc调用请求的字符流
    std::string recv_buf = buffer->retrieveAllAsString();

    // 使用 protobuf 的 CodedInputStream 来解析数据流
    google::protobuf::io::ArrayInputStream array_input(recv_buf.data(), recv_buf.size());
    google::protobuf::io::CodedInputStream coded_input(&array_input);
    uint32_t header_size{};

    coded_input.ReadVarint32(&header_size);  // 解析header_size

    // 根据 header_size 读取数据头的原始字符流，反序列化数据，得到rpc请求的详细信息
    std::string rpc_header_str;
    RPC::RpcHeader rpcHeader;
    std::string service_name;
    std::string method_name;

    // 设置读取限制，不必担心数据读多
    google::protobuf::io::CodedInputStream::Limit msg_limit = coded_input.PushLimit(header_size);
    coded_input.ReadString(&rpc_header_str, header_size);
    // 恢复之前的限制，以便安全地继续读取其他数据
    coded_input.PopLimit(msg_limit);
    uint32_t args_size{};
    if (rpcHeader.ParseFromString(rpc_header_str)) {
      // 数据头反序列化成功
      service_name = rpcHeader.service_name();
      method_name = rpcHeader.method_name();
      args_size = rpcHeader.args_size();
    } else {
      // 数据头反序列化失败
      std::cout << "rpc_header_str:" << rpc_header_str << " parse error!" << std::endl;
      return;
    }

    // 获取rpc方法参数的字符流数据
    std::string args_str;
    // 直接读取args_size长度的字符串数据
    bool read_args_success = coded_input.ReadString(&args_str, args_size);

    if (!read_args_success) {
      // 处理错误：参数数据读取失败
      return;
    }

    // 打印调试信息
    //    std::cout << "============================================" << std::endl;
    //    std::cout << "header_size: " << header_size << std::endl;
    //    std::cout << "rpc_header_str: " << rpc_header_str << std::endl;
    //    std::cout << "service_name: " << service_name << std::endl;
    //    std::cout << "method_name: " << method_name << std::endl;
    //    std::cout << "args_str: " << args_str << std::endl;
    //    std::cout << "============================================" << std::endl;

    // 获取service对象和method对象
    auto it = service_map_.find(service_name);
    if (it == service_map_.end()) {
      std::cout << "服务：" << service_name << " is not exist!" << std::endl;
      std::cout << "当前已经有的服务列表为:";
      for (auto item : service_map_) {
        std::cout << item.first << " ";
      }
      std::cout << std::endl;
      return;
    }

    auto mit = it->second.method_map_.find(method_name);
    if (mit == it->second.method_map_.end()) {
      std::cout << service_name << ":" << method_name << " is not exist!" << std::endl;
      return;
    }

    google::protobuf::Service *service = it->second.m_service;       // 获取service对象  new UserService
    const google::protobuf::MethodDescriptor *method = mit->second;  // 获取method对象  Login

    // 生成rpc方法调用的请求request和响应response参数,由于是rpc的请求，因此请求需要通过request来序列化
    google::protobuf::Message *request = service->GetRequestPrototype(method).New();
    if (!request->ParseFromString(args_str)) {
      std::cout << "request parse error, content:" << args_str << std::endl;
      return;
    }
    google::protobuf::Message *response = service->GetResponsePrototype(method).New();

    // 给下面的method方法的调用，绑定一个Closure的回调函数
    // closure是执行完本地方法之后会发生的回调，因此需要完成序列化和反向发送请求的操作
    google::protobuf::Closure *done =
        google::protobuf::NewCallback<RpcProvider, const muduo::net::TcpConnectionPtr &, google::protobuf::Message *>(
            this, &RpcProvider::SendRpcResponse, conn, response);

    // 在框架上根据远端rpc请求，调用当前rpc节点上发布的方法
    // new UserService().Login(controller, request, response, done)

    /*
    为什么这么写 service->CallMethod 就可以直接调用远程的业务方法?

    当你定义一个 .proto 文件，运行 protoc 编译器后，会生成一个服务类如 serviceRpc 类，这个类继承自 google::protobuf::Service
    接下来，用户需要定义自己的服务类 UserServiceRpc，并继承自 .protoc 生成的 serviceRpc 类

    用户注册的 UserServiceRpc 类里没有重写 google::protobuf::Service 的 CallMethod 方法，是 .protoc生成的 serviceRpc 类重写了
    而 .protoc 生成的 serviceRpc 类的会根据传入的 method 参数查找对应的服务方法，并调用用户在子类中重写的该方法
    由于 xx 方法被用户注册的 UserServiceRpc 类重写了，因此在运行时会调用这个方法

    即 CallMethod 函数内部会根据 method 描述符，动态调用对应的具体方法
    */
    service->CallMethod(method, nullptr, request, response, done);
}

/**
 * @brief 发送 RPC 响应消息的回调操作。
 *
 * 该方法用于将 Protobuf 消息序列化为字符串并通过网络发送回 RPC 调用方。
 * 它是 RPC 框架中处理响应的核心逻辑之一，确保调用方能够接收到服务端的执行结果。
 *
 * @param conn [输入] 指向当前 TCP 连接的智能指针。
 *             包含了连接的相关信息，例如对端地址、连接状态等。
 * @param response [输入] 指向 Protobuf 消息对象的指针，表示要发送的响应数据。
 *
 * @details
 * - 调用 `SerializeToString` 方法将 Protobuf 消息序列化为字符串。
 * - 如果序列化成功，通过 `conn->send()` 方法将序列化后的响应字符串发送给客户端。
 * - 如果序列化失败，输出错误日志以提示问题。
 * - 当前实现支持长连接模式，因此不会主动断开连接（原本的 `conn->shutdown()` 被注释掉）。
 *
 * @note
 * - 序列化失败可能是由于消息定义不正确或内存不足等原因导致。
 * - 长连接模式下，连接保持打开状态以便后续复用，避免频繁建立和断开连接的开销。
 *
 * @see google::protobuf::Message
 * @see muduo::net::TcpConnection
 */
void RpcProvider::SendRpcResponse(const muduo::net::TcpConnectionPtr &conn, google::protobuf::Message *response) {
    std::string response_str;
    if (response->SerializeToString(&response_str)) {
      conn->send(response_str);
    } else {
      std::cout << "serialize response_str error!" << std::endl;
    }
    // 模拟http的短链接服务，由 rpcprovider 主动断开连接
    // 改为长连接，不主动断开
    // conn->shutdown(); 
}

/**
 * @brief RpcProvider 的析构函数。
 *
 * 该析构函数负责清理 RPC 提供者的资源，并优雅地关闭事件循环。
 * 主要用于在服务端停止时释放相关资源并打印调试信息。
 *
 * @details
 * - 输出服务器绑定的 IP 和端口信息，方便调试和确认服务端配置。
 * - 调用 `m_eventLoop.quit()` 以停止事件循环，确保服务端能够安全退出。
 *
 * @note
 * - 析构函数通常由系统自动调用，无需手动触发。
 * - 确保在析构函数中释放所有动态分配的资源以避免内存泄漏。
 *
 * @see muduo::net::EventLoop
 */
RpcProvider::~RpcProvider() {
    std::cout << "[func - RpcProvider::~RpcProvider()]: ip 和 port 信息:" << muduo_server_->ipPort() << std::endl;
    m_eventLoop.quit();
}
