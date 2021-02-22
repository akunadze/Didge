#include "control.hpp"

volatile control::State state;
util::cached_value<uint16_t> control::rpm_cached;
util::cached_value<int16_t> control::encoder_cached;
util::cached_value<uint16_t> control::thread_index_cached;
util::cached_value<bool> control::mode_mm_cached;
util::cached_value<bool> control::mode_feed_cached;
util::cached_value<bool> control::mode_dir_cached;
util::cached_value<bool> control::stopped;

util::event control::pollEvent;

uint16_t control::last_tpi_index;
uint16_t control::last_mm_index;
uint16_t control::last_tpi_feed_index = 1;
uint16_t control::last_mm_feed_index = 1;
Configuration control::config;
