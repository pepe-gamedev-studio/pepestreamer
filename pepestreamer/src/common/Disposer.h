#pragma once
namespace pepestreamer {

template<typename Fn>
class Disposer {
  public:
    explicit Disposer(Fn fn) : dispose(fn) {}
    ~Disposer() { dispose(); }

  private:
    Fn dispose;
};

} // namespace pepestreamer
