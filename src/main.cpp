#include <coroutine>
#include <iostream>
#include <memory>
#include <vector>

// Generator coroutine type
template <typename T>
struct Generator {
  struct promise_type;
  using handle_type = std::coroutine_handle<promise_type>;

  struct promise_type {
    T value;
    std::suspend_always initial_suspend() { return {}; }
    std::suspend_always final_suspend() noexcept { return {}; }
    std::suspend_always yield_value(T v) {
      value = v;
      return {};
    }
    Generator get_return_object() {
      return Generator(handle_type::from_promise(*this));
    }
    void unhandled_exception() { std::exit(1); }
    void return_void() {}
  };

  handle_type coro;

  Generator(handle_type h) : coro(h) {}
  ~Generator() {
    if (coro)
      coro.destroy();
  }

  T getValue() { return coro.promise().value; }
  bool next() {
    coro.resume();
    return !coro.done();
  }
};

Generator<int> generate_numbers(int max) {
  for (int i = 0; i <= max; ++i) {
    co_yield i;
  }
}

struct ReturnObject {
  struct promise_type {
    ReturnObject get_return_object() { return {}; }
    std::suspend_never initial_suspend() { return {}; }
    std::suspend_never final_suspend() noexcept { return {}; }
    void unhandled_exception() {}
  };
};

struct Awaiter {
  std::coroutine_handle<>* hp_;
  constexpr bool await_ready() const noexcept { return false; }
  void await_suspend(std::coroutine_handle<> h) { *hp_ = h; }
  constexpr void await_resume() const noexcept {}
};

ReturnObject counter(std::coroutine_handle<>* continuation_out) {
  Awaiter a{continuation_out};
  for (unsigned i = 0;; ++i) {
    co_await a;
    std::cout << "counter: " << i << std::endl;
  }
}

void main1() {
  std::coroutine_handle<> h;
  counter(&h);
  for (int i = 0; i < 3; ++i) {
    std::cout << "In main1 function\n";
    h();
  }
  h.destroy();
}

int main() {
#if 0
  auto numbers = generate_numbers(10);
  while (numbers.next()) {
    std::cout << "Generated number: " << numbers.getValue() << std::endl;
  }
#else
  main1();
#endif
  return 0;
}
