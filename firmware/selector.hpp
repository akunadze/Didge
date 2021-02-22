#include "mcu.hpp"
#include "control.hpp"

namespace selector {
    using pin_chA = mcu::pins::selector_A;
    using pin_chB = mcu::pins::selector_B;

    static uint8_t _state = 0;
    static int16_t counter = 0;
    static int16_t max = 9999;
    static int16_t min = 0;

    static void onThreadChange(uint16_t newIndex) {
        counter = std::min(max, std::max(min, (int16_t)newIndex));
    }

    static void setLimits(int16_t _min, int16_t _max) {
        min = _min;
        max = _max;

        counter = std::min(max, std::max(min, counter));
    }

    static void onModeChange(bool) {
        uint16_t min, max;
        control::config.get_limits(min, max, control::mode_feed_cached.get(), control::mode_mm_cached.get());
        
        setLimits(min, max);
    }

    static void init() {
        using namespace Kvasir;

        apply(write(pin_chA::cr::cnf, gpio::PinConfig::Input_pullup_pulldown),
            set(pin_chA::bsrr),
            write(pin_chB::cr::cnf, gpio::PinConfig::Input_pullup_pulldown),
            set(pin_chB::bsrr));

        apply(write(Tim4Cr1::ckd, 0),
            write(Tim4Cr1::cms, 0),
            set(Tim4Cr1::arpe),
            write(Tim4Arr::arr, 100),
            write(Tim4Psc::psc, 1000),
            set(Tim4Egr::ug),
            clear(Tim4Sr::uif),
            set(Tim4Dier::uie));

        mcu::enable_interrupt<IRQ::tim4_irqn>();
        apply(set(Tim4Cr1::cen)); // enable timer

        control::thread_index_cached.subscribe(onThreadChange);
        control::mode_mm_cached.subscribe(onModeChange);
        control::mode_feed_cached.subscribe(onModeChange);

        onModeChange(true);
    }

    #define R_CW_FINAL 0x1
    #define R_CW_BEGIN 0x2
    #define R_CW_NEXT 0x3
    #define R_CCW_BEGIN 0x4
    #define R_CCW_FINAL 0x5
    #define R_CCW_NEXT 0x6

    #define DIR_NONE 0x00
    #define DIR_CW 0x10
    #define DIR_CCW 0x20

    #define R_START 0x0

    const unsigned char ttable[][4] =
    {
        // 00         01           10           11
        {R_START, R_CW_BEGIN, R_CCW_BEGIN, R_START},           // R_START
        {R_CW_NEXT, R_START, R_CW_FINAL, R_START | DIR_CW},    // R_CW_FINAL
        {R_CW_NEXT, R_CW_BEGIN, R_START, R_START},             // R_CW_BEGIN
        {R_CW_NEXT, R_CW_BEGIN, R_CW_FINAL, R_START},          // R_CW_NEXT
        {R_CCW_NEXT, R_START, R_CCW_BEGIN, R_START},           // R_CCW_BEGIN
        {R_CCW_NEXT, R_CCW_FINAL, R_START, R_START | DIR_CCW}, // R_CCW_FINAL
        {R_CCW_NEXT, R_CCW_FINAL, R_CCW_BEGIN, R_START}        // R_CCW_NEXT
    };

    static void update() {
        uint8_t a = apply(read(pin_chA::idr));
        uint8_t b = apply(read(pin_chB::idr));

        uint8_t pinstate = (a << 1) | b;

        _state = ttable[_state & 0xf][pinstate];

        uint8_t result = _state & 0x30;
        if (result && control::canChangeIndex()) {
            counter += (result == DIR_CW ? 1 : -1);
            if (counter > max)
                counter = max;
            else if (counter < min)
                counter = min;
        }
    }
}
