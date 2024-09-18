#include <boost/asio.hpp>
#include <future>
#include <iostream>
#include <thread>

using boost::asio::ip::tcp;

// 复制数据
void copy_data(tcp::socket& src_socket, tcp::socket& dest_socket) {
  try {
    char buffer[1024];
    boost::system::error_code ec;

    while (true) {
      // 从源套接字读取数据
      size_t bytes_read = src_socket.read_some(boost::asio::buffer(buffer), ec);
      if (ec == boost::asio::error::eof) {
        break;  // 连接关闭
      } else if (ec) {
        throw boost::system::system_error(ec);
      }

      // 将数据写入目标套接字
      boost::asio::write(dest_socket, boost::asio::buffer(buffer, bytes_read),
                         ec);
      if (ec) {
        throw boost::system::system_error(ec);
      }
    }
  } catch (const std::exception& e) {
    std::cerr << "Exception in copy_data: " << e.what() << "\n";
  }
}

// 处理客户端连接
void handle_client(tcp::socket client_socket,
                   const std::string& forward_address) {
  try {
    boost::asio::io_context io_context;  // 创建一个新的 io_context 实例
    tcp::socket server_socket(
        io_context);  // 使用 io_context 实例创建 server_socket

    // 解析目标地址
    tcp::resolver resolver(io_context);
    auto endpoints = resolver.resolve(forward_address, "5555");

    // 连接到服务器
    boost::asio::connect(server_socket, endpoints);

    // 异步复制数据
    std::future<void> to_server =
        std::async(std::launch::async, copy_data, std::ref(client_socket),
                   std::ref(server_socket));
    std::future<void> to_client =
        std::async(std::launch::async, copy_data, std::ref(server_socket),
                   std::ref(client_socket));

    // 等待两个异步任务完成
    to_server.get();
    to_client.get();

  } catch (const std::exception& e) {
    std::cerr << "Exception in handle_client: " << e.what() << "\n";
  }

  client_socket.close();
}

int main() {
  try {
    boost::asio::io_context io_context;

    // 创建并启动监听端口
    tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(), 5556));
    std::cout << "Listening on port 5556, forwarding to 127.0.0.1:5555\n";

    while (true) {
      // 接受新的连接
      tcp::socket socket(io_context);
      acceptor.accept(socket);

      std::cout << "Accepted connection from " << socket.remote_endpoint()
                << "\n";

      // 在新线程中处理连接
      std::thread(handle_client, std::move(socket), "127.0.0.1").detach();
    }

  } catch (const std::exception& e) {
    std::cerr << "Exception: " << e.what() << "\n";
  }

  return 0;
}