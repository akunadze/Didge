#pragma once

#include <optional>
#include "threads.hpp"
#include "thread_list.hpp"

struct Configuration {
  using gearing_ratio_t = std::pair<uint16_t, uint16_t>;
  
  uint16_t encoder_resolution{4096u};
  gearing_ratio_t encoder_gearing{1, 1};
  
  uint16_t stepper_full_steps{200u};
  uint16_t stepper_micro_steps{16};
  gearing_ratio_t stepper_gearing{1, 1};
  
  unsigned step_pulse_ns{1200};
  unsigned step_dir_hold_ns{400};
  bool invert_step_pin{false};
  bool invert_dir_pin{true};
  
  using Rational = threads::Rational;
  
  Rational leadscrew_pitch{threads::tpi_pitch(15)};

  threads::thread thread = threads::tpi_list[0];
  threads::thread feed_thread;

  const uint16_t max_feed_mm = 250;
  const uint16_t max_feed_tpi = 250;

  Configuration() {
    rationals.encoder = {encoder_resolution * encoder_gearing.first, encoder_gearing.second};
    rationals.steps_per_rev = {stepper_full_steps * stepper_micro_steps *
      stepper_gearing.first, stepper_gearing.second};
  }
  
  Rational calculate_ratio() const {
    return calculate_ratio_for_pitch(thread.pitch.value);
  }

  void select_thread(int16_t new_pitch_index, bool mm) {
    thread = mm ? threads::mm_list[new_pitch_index] : threads::tpi_list[new_pitch_index];
  }

  void set_feed(uint16_t value, bool mm) {
    feed_thread.pitch = threads::detail::make_pitch_info("", mm ? 100 : 1000, value, mm ? threads::pitch_type::mm : threads::pitch_type::tpi);
    thread = feed_thread;
  }
  
  void get_limits(uint16_t &min, uint16_t &max, bool feed, bool mm) {
    if (feed) {
      min = 1;
      max = 250;
    } else {
      min = 0;
      max = mm ? threads::mm_list_size - 1 : threads::tpi_list_size - 1;
    }
  }

  enum thread_compatibility : uint8_t {
    thread_OK = 0,
    thread_too_large,
    thread_too_small
  };
  
  template <typename TimerCounter = uint16_t>
  thread_compatibility verify_thread() const {
    auto ratio = calculate_ratio_for_pitch(thread.pitch.value);
    auto n = ratio.numerator(), d = ratio.denominator();
    if (n >= d) {
      return thread_too_large;
    }
    if (((d + n - 1) / n) > (std::numeric_limits<TimerCounter>::max() / 2)) {
      return thread_too_small;
    }
    return thread_OK;
  }

private:
  struct Bundle {
    Rational encoder{};
    Rational steps_per_rev{};
  };
  
  Bundle rationals{};

  Rational calculate_ratio_for_pitch(const Rational& pitch) const {
    return (pitch / leadscrew_pitch) * rationals.steps_per_rev / rationals.encoder;
  }
  
};

