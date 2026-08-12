#include <functional>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

namespace {

using CompletionCallback = std::function<void(bool)>;

class Dispatcher {
public:
  void post(std::function<void()> callback) {
    callbacks_.push_back(std::move(callback));
  }

  void run() {
    for (auto &callback : callbacks_) {
      callback();
    }
    callbacks_.clear();
  }

private:
  std::vector<std::function<void()>> callbacks_;
};

void sendAsync(Dispatcher &dispatcher, std::string request,
               CompletionCallback on_complete) {
  // callback 可能晚于本函数执行，因此按值捕获（并 move）所需数据。
  dispatcher.post(
      [request = std::move(request), callback = std::move(on_complete)]() {
        std::cout << "send: " << request << '\n';
        callback(true);
      });
}

} // namespace

int main() {
  Dispatcher dispatcher;
  int completed = 0;
  sendAsync(dispatcher, "GET /ready", [&completed](bool ok) {
    if (ok) {
      ++completed;
    }
  });
  dispatcher.run();
  std::cout << "completed=" << completed << '\n';
}
