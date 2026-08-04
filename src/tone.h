#pragma once
#include <raylib.h>
#include <cstdint>

using i16 = std::int16_t;



struct Tone {
  static constexpr float duration = 0.1f;
  static constexpr int sample_rate = 22'000;
  static constexpr int sample_count = duration * sample_rate;

  float freq;
  i16 samples[sample_count];

  Tone(float freq);
  
  auto to_raylib() -> Sound;
};

