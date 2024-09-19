#include <boost/asio.hpp>
#include <exception>
#include <iostream>

using boost::asio::ip::tcp;

// 复制数据
boost::asio::awaitable<void> copy_data(tcp::socket& src_socket,
                                       tcp::socket& dest_socket,
                                       const char* tag) {
  char buffer[1024];

  while (true) {
    size_t bytes_read, bytes_written;
    bytes_read = co_await src_socket.async_receive(
        boost::asio::buffer(buffer, sizeof(buffer)),
        boost::asio::use_awaitable);
#if defined(ENABLE_VERBOSE_LOGGING)
    std::cerr << "Read data " << bytes_read << " bytes for " << tag
              << std::endl;
#endif
    if (bytes_read == 0) {
      break;  // 连接关闭
    }

    bytes_written = co_await dest_socket.async_send(
        boost::asio::buffer(buffer, bytes_read), boost::asio::use_awaitable);
#if defined(ENABLE_VERBOSE_LOGGING)
    std::cerr << "Written data " << bytes_written << " bytes for " << tag
              << std::endl;
#endif
  }
}

// 处理客户端连接
boost::asio::awaitable<void> handle_client(boost::asio::io_context& io_context,
                                           tcp::socket client_socket,
                                           const std::string& forward_address) {
  try {
    tcp::socket server_socket(
        io_context);  // 使用 io_context 实例创建 server_socket

    // 解析目标地址
    tcp::resolver resolver(io_context);
    auto endpoints = resolver.resolve(forward_address, "5555");

    // 连接到服务器
#if defined(ENABLE_VERBOSE_LOGGING)
    std::cout << "Connecting to server...\n";
#endif
    co_await boost::asio::async_connect(server_socket, endpoints,
                                        boost::asio::use_awaitable);
#if defined(ENABLE_VERBOSE_LOGGING)
    std::cout << "Connected to server.\n";
#endif

    // 启动协程任务
    auto c0 = copy_data(client_socket, server_socket, "client to server");
    auto c1 = copy_data(server_socket, client_socket, "server to client");
    boost::asio::co_spawn(io_context, std::move(c0), boost::asio::detached);
    co_await std::move(c1);
  } catch (const std::exception& e) {
    std::cerr << "Exception in handle_client: " << e.what() << "\n";
  }

  client_socket.close();
}

// 主协程
boost::asio::awaitable<void> main_coroutine(boost::asio::io_context& io_context,
                                            tcp::acceptor& acceptor) {
  while (true) {
    tcp::socket socket(io_context);

    // 异步接受连接
    co_await acceptor.async_accept(socket, boost::asio::use_awaitable);

    std::cout << "Accepted connection from " << socket.remote_endpoint()
              << "\n";

    // 启动处理客户端连接的协程
    boost::asio::co_spawn(
        io_context, handle_client(io_context, std::move(socket), "127.0.0.1"),
        boost::asio::detached);
  }
}

int main() {
  try {
    boost::asio::io_context io_context;

    // 创建并启动监听端口
    tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(), 5556));
    std::cout << "Listening on port 5556, forwarding to 127.0.0.1:5555\n";

    // 启动主协程
    boost::asio::co_spawn(io_context, main_coroutine(io_context, acceptor),
                          boost::asio::detached);

    // 运行 io_context
    io_context.run();

  } catch (const std::exception& e) {
    std::cerr << "Exception: " << e.what() << "\n";
  }

  return 0;
}