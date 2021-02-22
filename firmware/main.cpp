#include <limits>
#include <algorithm>

// Kvasir imports
#include <Chip/STM32F103xx.hpp>
#include <Register/Register.hpp>
#include <Register/Utility.hpp>

#include "mcu.hpp"
#include "devices.hpp"
#include "gear.hpp"
#include "threads.hpp"
#include "thread_list.hpp"
#include "common.hpp"
#include "control.hpp"
#include "MAX7219.hpp"
#include "switches.hpp"
#include "selector.hpp"

namespace systick_state {
  volatile uint8_t rpm_sample_prescale_count = 0;
}

volatile bool rpm_update = false;
volatile bool rpm_report = false;

extern "C" { // interrupt handlers
  void SysTick_Handler() { // Called every 1 ms
    using rpm_sampler = devices::rpm_counter<>;
    using namespace systick_state;
    if (rpm_report) {
      auto psc = rpm_sample_prescale_count;
      if (++psc == rpm_sampler::Sampling_period) {
        if (rpm_sampler::process_sample(devices::encoder::get_count())) {
          rpm_update = true;
        }
        rpm_sample_prescale_count = 0;
      }
      else {
        rpm_sample_prescale_count = psc;
      }
    }  
    if (((++mcu::milliseconds) & 1023) == 0) {
      //mcu::toggle_led();
    }
  }

  void TIM1_CC_IRQHandler() {
    using namespace devices;
    auto enc = encoder::get_count();
    bool dir = step_gen::get_direction();
    const bool fwd = encoder::is_cc_fwd_interrupt();
    encoder::clear_cc_interrupt();
    using namespace gear;
    if (nextExists) {
      state.D = nextState.D;
      state.N = nextState.N;
      state.err = 0;

      nextExists = false;
      range.next.error = range.prev.error = 0;
    }
    
    if (fwd) {
      encoder::trigger_clear();
      state.err = range.next.error;
      range.next_jump(dir, enc);
      step_gen::set_delay(phase_delay(encoder_pulse_duration::last_duration(), range.next.error));
      encoder::trigger_restore();
    }
    else { // Change direction, setup delayed pulse and do manual trigger     
      //mcu::toggle_led();
      dir = !dir;
      step_gen::change_direction(dir);
      encoder::trigger_manual_pulse();
      state.err = range.prev.error;
      range.next_jump(dir, enc);
    }
    encoder::update_channels(range.next.count, range.prev.count);
  }

  void TIM2_IRQHandler() {
    devices::encoder_pulse_duration::process_interrupt();
  }

  void TIM3_IRQHandler() {
    using devices::step_gen;
    step_gen::process_interrupt();
    gear::state.output_position += step_gen::get_direction() ? 1 : -1;
  }

  void TIM4_IRQHandler() {
    selector::update();
    apply(clear(Kvasir::Tim4Sr::uif));
  }

  void USART1_IRQHandler() {
  }
} // extern "C"

void onThreadChange(uint16_t newIndex) {
  using namespace Kvasir;

  auto t = control::config.verify_thread();
  if (t != Configuration::thread_OK) {
      mcu::toggle_led();
  }
  
  auto pr = control::config.calculate_ratio();
  gear::configure(pr, devices::encoder::get_count(), devices::rpm_counter<>::get_rpm(control::config.encoder_resolution) == 0);
}

int main() {
  using namespace devices;

  mcu::init();
  control::thread_index_cached.subscribe(onThreadChange);

  //Serial2<>::init(); // Used as console
  
  step_gen::init();
  step_gen::configure(control::config.step_dir_hold_ns, control::config.step_pulse_ns, 
          control::config.invert_step_pin, control::config.invert_dir_pin);
  
  control::thread_index_cached.forceNotify(0);

  encoder::init();
  encoder::update_channels(gear::range.next.count, gear::range.prev.count);
  
  encoder_pulse_duration::init();

  //ledkey::init();  
  selector::init();
  max7219::init();
  switches::init();
  
  rpm_report = true;
  
  while (true) {
    control::pollEvent.fire();

    if (rpm_update && rpm_report) {
      rpm_update = false;
      control::rpm_cached = rpm_counter<>::get_rpm(control::config.encoder_resolution); 
      control::stopped = (control::rpm_cached.get() == 0);
    }
    
    control::encoder_cached = selector::counter;
  }
  
  return 0;
}
