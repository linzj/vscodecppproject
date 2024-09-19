#include <boost/asio.hpp>
#include <boost/asio/experimental/as_tuple.hpp>
#include <boost/asio/experimental/awaitable_operators.hpp>
#include <boost/asio/experimental/co_spawn.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/asio/write.hpp>
#include <iostream>

namespace asio = boost::asio;
using asio::ip::tcp;
using namespace asio::experimental::awaitable_operators;

constexpr uint16_t listen_port = 5556;
constexpr char forward_host[] = "127.0.0.1";
constexpr uint16_t forward_port = 5555;
// #define ENABLE_VERBOSE_LOG 1

asio::awaitable<void> forward_data(tcp::socket& client,
                                   tcp::socket& server,
                                   const char* tag) {
  try {
    std::vector<char> data(1024 * 1024);
    while (true) {
      size_t n = co_await client.async_receive(asio::buffer(data),
                                               asio::use_awaitable);
#if defined(ENABLE_VERBOSE_LOG)
      std::cerr << "received " << n << " bytes data for " << tag << std::endl;
#endif
      size_t sent = co_await boost::asio::async_write(
          server, asio::buffer(data, n), asio::use_awaitable);
#if defined(ENABLE_VERBOSE_LOG)
      std::cerr << "sent " << sent << " bytes data for " << tag << std::endl;
#endif
    }
  } catch (std::exception& e) {
    std::cerr << "Forwarding failed: " << e.what() << '\n';
  }
}

asio::awaitable<void> handle_session(tcp::socket client) {
  try {
    tcp::resolver resolver(client.get_executor());
    auto endpoints = co_await resolver.async_resolve(
        forward_host, std::to_string(forward_port), asio::use_awaitable);
    tcp::socket server(client.get_executor());
    co_await server.async_connect(*endpoints.begin(), asio::use_awaitable);

    auto client_to_server = forward_data(client, server, "client to server");
    auto server_to_client = forward_data(server, client, "server to client");

    co_await (std::move(client_to_server) && std::move(server_to_client));
  } catch (std::exception& e) {
    std::cerr << "Session error: " << e.what() << '\n';
  }
}

asio::awaitable<void> listener(asio::io_context& io_context) {
  auto executor = co_await asio::this_coro::executor;
  tcp::acceptor acceptor(executor, tcp::endpoint(tcp::v6(), listen_port));
  acceptor.set_option(tcp::acceptor::reuse_address(true));

  while (true) {
    tcp::socket socket = co_await acceptor.async_accept(asio::use_awaitable);
    asio::co_spawn(executor, handle_session(std::move(socket)), asio::detached);
  }
}

int main() {
  try {
    asio::io_context io_context;
    asio::signal_set signals(io_context, SIGINT, SIGTERM);
    signals.async_wait([&](auto, auto) { io_context.stop(); });

    asio::co_spawn(io_context, listener(io_context), asio::detached);
    io_context.run();
  } catch (std::exception& e) {
    std::cerr << "Main error: " << e.what() << '\n';
  }
  return 0;
}