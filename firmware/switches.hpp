#pragma once

#include "mcu.hpp"
#include "control.hpp"

namespace switches {
    using pin_mm = mcu::pins::switch_mm;
    using pin_feed = mcu::pins::switch_feed;
    using pin_dir = mcu::pins::switch_dir;

    static void poll() {
        uint8_t mm = apply(read(pin_mm::idr));
        control::setMmMode(mm);

        uint8_t feed = apply(read(pin_feed::idr));
        control::setFeedMode(feed);
    }

    static void init() {
      using namespace Kvasir;

      apply(write(pin_mm::cr::cnf, gpio::PinConfig::Input_pullup_pulldown),
            write(pin_mm::cr::mode, gpio::PinMode::Input),
            write(pin_feed::cr::cnf, gpio::PinConfig::Input_pullup_pulldown),
            write(pin_feed::cr::mode, gpio::PinMode::Input),
            write(pin_dir::cr::cnf, gpio::PinConfig::Input_pullup_pulldown),
            write(pin_dir::cr::mode, gpio::PinMode::Input));

      control::pollEvent.subscribe(poll);
    }
}