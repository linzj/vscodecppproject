#include <boost/asio.hpp>
#include <iostream>
#include <random>

using boost::asio::awaitable;
using boost::asio::co_spawn;
using boost::asio::steady_timer;
using namespace std::chrono_literals;

// Function to delay and then generate a random number
awaitable<int> delayed_random_number(boost::asio::io_context& io) {
  steady_timer timer(io, 5s);
  co_await timer.async_wait(boost::asio::use_awaitable);

  // Generate random number after delay
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<> distrib(1, 100);
  int result = distrib(gen);

  co_return result;
}

int main() {
  boost::asio::io_context io_context;

  // Launch the asynchronous operation
  co_spawn(io_context, delayed_random_number(std::ref(io_context)),
           [](std::exception_ptr e, int result) {
             if (e) {
               try {
                 std::rethrow_exception(e);
               } catch (const std::exception& ex) {
                 std::cerr << "Caught exception: " << ex.what() << '\n';
               }
             } else {
               std::cout << "Random number: " << result << '\n';
             }
           });

  // Run the io_context to perform the asynchronous operations
  io_context.run();

  return 0;
}