#pragma once

#include "common.hpp"
#include "configuration.hpp"

class control {
    public:

    enum class State : uint8_t {
        stopped,
        in_sync,
        ramping,
    };
    
    static volatile State state;

    static util::cached_value<uint16_t> rpm_cached;
    static util::cached_value<int16_t> encoder_cached;
    static util::cached_value<uint16_t> thread_index_cached;
    static util::cached_value<bool> mode_mm_cached;
    static util::cached_value<bool> mode_feed_cached;
    static util::cached_value<bool> mode_dir_cached;
    static util::cached_value<bool> stopped;
    
    static util::event pollEvent;

    static uint16_t last_tpi_index;
    static uint16_t last_mm_index;
    static uint16_t last_tpi_feed_index;
    static uint16_t last_mm_feed_index;

    static Configuration config;

    static void changeThread(uint16_t newIndex) {
        // This should never happen (UI should check before changing index)
        // Just to be safe, check it here as well
        if (!canChangeIndex()) return;
 
        // TODO:
        // Acceleration:
        // find out current and target output speed
        //   check for target speed too high -> trigger stop
        //      if speeds are not significantly different do nothing
        //   if the state is stopped current speed is zero, 
        //    if ramping -> re-adjust target speed
        // setup acceleration settings in acceleration device
        // switch step_gen to trigger from the accelerator
        if (mode_feed_cached.get()) {
            control::config.set_feed(newIndex, mode_mm_cached.get());
        } else {
            control::config.select_thread(newIndex, mode_mm_cached.get());
        }
        
        thread_index_cached.forceNotify(newIndex);
    }

    static uint16_t *getSavedIndexVar() {
        if (mode_feed_cached.get()) {
            if (mode_mm_cached.get()) {
                return &last_mm_feed_index;
            } else {
                return &last_tpi_feed_index;
            }
        } else {
            if (mode_mm_cached.get()) {
                return &last_mm_index;
            } else {
                return &last_tpi_index;
            }
        }
    }

    static void setMmMode(bool mm) {
        if (mode_mm_cached.get() != mm) {
            *(getSavedIndexVar()) = thread_index_cached.get();
            mode_mm_cached = mm;
            changeThread(*(getSavedIndexVar()));
        }
    }

    static void setFeedMode(bool feed) {
        if (!feed && !stopped.get()) {
            // don't allow switching to thread mode while spindle is in motion
            return;
        }
        
        if (mode_feed_cached.get() != feed) {
            *(getSavedIndexVar()) = thread_index_cached.get();
            mode_feed_cached = feed;
            changeThread(*(getSavedIndexVar()));
        }
    }

    static bool canChangeIndex() {
        return (stopped.get() || mode_feed_cached.get());
    }
};
