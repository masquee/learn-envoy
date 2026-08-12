#include <iostream>
#include <memory>
#include <string>

namespace {

class Connection {
public:
  explicit Connection(std::string name) : name_(std::move(name)) {
    std::cout << "open " << name_ << '\n';
  }

  ~Connection() { std::cout << "close " << name_ << '\n'; }

  void send() const { std::cout << "send via " << name_ << '\n'; }

private:
  std::string name_;
};

std::unique_ptr<Connection> createConnection() {
  return std::make_unique<Connection>("upstream");
}

} // namespace

int main() {
  // unique_ptr 表达唯一所有权，离开 scope 时自动析构。
  std::unique_ptr<Connection> connection = createConnection();
  connection->send();

  // shared_ptr 表达共享所有权；weak_ptr 观察对象但不延长生命周期。
  auto shared = std::make_shared<Connection>("listener");
  std::weak_ptr<Connection> observer = shared;
  if (std::shared_ptr<Connection> alive = observer.lock()) {
    alive->send();
  }
  shared.reset();
  std::cout << "listener alive=" << std::boolalpha << !observer.expired()
            << '\n';
}
