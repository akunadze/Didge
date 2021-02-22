#pragma once

#include <optional>
#include <vector>

namespace util {

  class event {
    typedef void (*NotifyType)();
    std::vector<NotifyType> funcs;

  public:
    void fire() {
      for (auto f: funcs) {
        f();
      }
    }

    void subscribe(NotifyType f) {
      funcs.push_back(f);
    }
  };

  template <typename PodType>
  class cached_value {
    volatile PodType master{};
    std::optional<PodType> copy;

    typedef void (*NotifyType)(PodType);
    std::vector<NotifyType> funcs;

  public:
    cached_value& operator= (PodType new_value) {
      master = new_value;

      if (copy != master) {
        for (auto f: funcs) {
          copy = master;
          f(master);
        }
      }
      return *this;
    }
    
    void forceNotify(PodType new_value) {
      master = new_value;

      for (auto f: funcs) {
        copy = master;
        f(master);
      }
    }

    void subscribe(NotifyType f) {
      funcs.push_back(f);
    }

    PodType get() {
      return master;
    }
  };


#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wnarrowing"
  // C/C++ arithmetic operations are promoted to integral types by default
  // which makes shorter integer operations emit warning messages.
  // These meta-functions provide a warning free path to do arithmetic operations
  // on shorter types. Motivation is that the instructions for shorter integers
  // are more efficient (instructions sizes are smaller).
  // Assumes overflow/underflow does not occur or matter!
  template <typename T>
  constexpr T narrow(unsigned value) {
    return static_cast<T>(value);
  }
  
  template <typename T>
  constexpr T narrow(int value) {
    return static_cast<T>(value);
  }
  
#pragma GCC diagnostic pop
  
}