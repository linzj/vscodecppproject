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

template <typename Executor = boost::asio::any_io_executor>
class InitiateAsyncNum {
 public:
  typedef Executor executor_type;
  explicit InitiateAsyncNum(Executor executor) : executor_(executor) {}

  template <typename CompleteHandler>
  void operator()(CompleteHandler&& handler) {
    (void)std::async(std::launch::async,
                     [my_handler = std::move(handler),
                      my_executor = std::move(executor_)]() mutable {
                       std::random_device rd;
                       std::mt19937 gen(rd());
                       std::uniform_int_distribution<> distrib(1, 100);
                       int result = distrib(gen);
                       boost::asio::dispatch(
                           my_executor,
                           [result, handler = std::move(my_handler)]() mutable {
                             std::move(handler)(result);
                           });
                     });
  }

  executor_type executor_;
};

template <typename Executor = boost::asio::any_io_executor>
struct AsyncNum {
  typedef Executor executor_type;

  explicit AsyncNum(executor_type executor) : executor_(executor) {}
  ~AsyncNum() = default;

  template <BOOST_ASIO_COMPLETION_TOKEN_FOR(void(int)) ReadToken =
                boost::asio::default_completion_token_t<executor_type>>
  auto async_num(ReadToken&& token =
                     boost::asio::default_completion_token_t<executor_type>())
      -> decltype(async_initiate<ReadToken, void(int)>(
          std::declval<InitiateAsyncNum<Executor>>(),
          token)) {
    return async_initiate<ReadToken, void(int)>(
        InitiateAsyncNum<Executor>(executor_), token);
  }

  executor_type executor_;
};

awaitable<int> delayed_random_number_async(boost::asio::io_context& io) {
  auto executor = co_await boost::asio::this_coro::executor;
  AsyncNum async_num(executor);
  co_return co_await async_num.async_num(boost::asio::use_awaitable);
}

awaitable<void> InitiateCoSpawn(boost::asio::io_context& io) {
  int result;
  try {
    result = co_await co_spawn(io, delayed_random_number(io),
                               boost::asio::use_awaitable);
  } catch (std::exception& ex) {
    std::cerr << "Caught exception: " << ex.what() << '\n';
  }

  std::cout << "Random number async co spawn: " << result << '\n';
}

int main() {
  boost::asio::io_context io_context;
  boost::asio::io_context io_context_async;
  auto work_guard = boost::asio::make_work_guard(io_context_async);

  auto async_future = std::async(
      std::launch::async, [&io_context_async]() { io_context_async.run(); });

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

  co_spawn(io_context, delayed_random_number_async(std::ref(io_context)),
           [](std::exception_ptr e, int result) {
             if (e) {
               try {
                 std::rethrow_exception(e);
               } catch (const std::exception& ex) {
                 std::cerr << "Caught exception: " << ex.what() << '\n';
               }
             } else {
               std::cout << "Random number async: " << result << '\n';
             }
           });
  co_spawn(io_context, InitiateCoSpawn(io_context_async),
           boost::asio::detached);

  // Run the io_context to perform the asynchronous operations
  io_context.run();
  io_context_async.stop();
  async_future.get();

  return 0;
}