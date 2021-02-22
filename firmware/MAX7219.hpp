#pragma once

#include "mcu.hpp"

namespace max7219 {
    using pin_clk = mcu::pins::display_clk;
    using pin_cs = mcu::pins::display_cs;
    using pin_miso = mcu::pins::display_miso;

    typedef enum {
        REG_NO_OP 			= 0x00,
        REG_DIGIT_0 		= 0x01,
        REG_DIGIT_1 		= 0x02,
        REG_DIGIT_2 		= 0x03,
        REG_DIGIT_3 		= 0x04,
        REG_DIGIT_4 		= 0x05,
        REG_DIGIT_5 		= 0x06,
        REG_DIGIT_6 		= 0x07,
        REG_DIGIT_7 		= 0x08,
        REG_DECODE_MODE 	= 0x09,
        REG_INTENSITY 		= 0x0A,
        REG_SCAN_LIMIT 		= 0x0B,
        REG_SHUTDOWN 		= 0x0C,
        REG_DISPLAY_TEST 	= 0x0F,
    } MAX7219_REGISTERS;

    const uint8_t blank = 0xF;
    const uint8_t dot = 0xF0;

    static void sendByte(uint8_t data) {
        using namespace Kvasir;

        apply(write(Spi2Dr::dr, data));
        while(!apply(read(Spi2Sr::txe))) {}
        while(apply(read(Spi2Sr::bsy))) {}
    }

    static void sendData(uint8_t addr, uint8_t data) {
        using namespace Kvasir;

        apply(write(pin_cs::odr, false));

        sendByte(addr);
        sendByte(data);

        apply(write(pin_cs::odr, true));
    }

    static void displayInt(int num, int position) {
        int startpos, endpos;

        switch (position) {
            case 0: startpos = 1;
                    endpos = 8;
                    break;
            case 1: startpos = 1;
                    endpos = 4;
                    break;
            case 2: startpos = 5;
                    endpos = 8;
        }

        int mod = 1;
        for (int i = startpos; i <= endpos; i++) {
            sendData(i, num % (mod * 10) / mod);
            mod = mod * 10;
        }
    }

    static void displayText(const char *text, int position) {
        int startpos, endpos;

        switch (position) {
            case 0: startpos = 8;
                    endpos = 1;
                    break;
            case 1: startpos = 4;
                    endpos = 1;
                    break;
            case 2: startpos = 8;
                    endpos = 5;
        }

        const char *p = text;
        for (int i = startpos; i >= endpos; i--) {
            if (*p == '.') p++;
            
            if (*p) {
                if (*p >= '0' && *p <= '9') {
                    sendData(i, (*p - '0') | (p[1] == '.' ? dot : 0));
                } else {
                    sendData(i, blank);
                }
                p++;
            } else {
                sendData(i, blank);
            }
        }
    }

    static void onEncoderChange(int16_t newValue) {
        control::changeThread(newValue);
    }

    static void onThreadChange(uint16_t newIndex) {
        char buff[6];

        if (control::mode_feed_cached.get()) {
            const char *fmt;
            if (control::mode_mm_cached.get()) {
                fmt = " %d.%d%d";
            } else {
                fmt = "0.%d%d%d";
            }
            snprintf(buff, 6, fmt, newIndex / 100, newIndex % 100 / 10, newIndex % 10);
        } else {
            const char *pitch = control::config.thread.pitch.pitch_str.data();
            if (strchr(pitch, '.')) {
                snprintf(buff, 6, "%5s", pitch); 
            } else {
                snprintf(buff, 6, "%4s", pitch); 
            }
        }

        displayText(buff, 1);
    }

    static void onRpmChange(uint16_t newValue) {
        displayInt(newValue, 2);
    }

    static void init() {
        using namespace Kvasir;

        apply(write(pin_clk::cr::cnf, gpio::PinConfig::Output_alternate_push_pull));
        apply(write(pin_clk::cr::mode, gpio::PinMode::Output_50Mhz));
        apply(write(pin_miso::cr::cnf, gpio::PinConfig::Output_alternate_push_pull));
        apply(write(pin_miso::cr::mode, gpio::PinMode::Output_50Mhz));

        apply(write(pin_cs::cr::cnf, gpio::PinConfig::Output_push_pull),
              write(pin_cs::cr::mode, gpio::PinMode::Output_10Mhz));

        apply(write(Spi2Cr1::br, 3),
              write(Spi2Cr1::ssm, 1),
              write(Spi2Cr1::ssi, 1),
              write(Spi2Cr1::mstr, 1),
              write(Spi2Cr1::spe, 1));

        sendData(REG_SHUTDOWN, 0x01);
        sendData(REG_SCAN_LIMIT, 7);
        sendData(REG_INTENSITY, 0x10);
        sendData(REG_DECODE_MODE, 0xFF);
        
        for (int i = 1; i <= 8; i++) {
            sendData(i, blank);
        }

        control::rpm_cached.subscribe(onRpmChange);
        control::encoder_cached.subscribe(onEncoderChange);
        control::thread_index_cached.subscribe(onThreadChange);
    }
}