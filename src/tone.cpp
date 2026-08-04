#include "tone.h"
#include <cmath>



Tone::Tone(float freq) : freq{freq} {
  for (int i{}; i < Tone::sample_count; ++i) {
    float t = static_cast<float>(i) / sample_rate;
    if (std::sinf(2 * PI * freq * t) >= 0) {
      samples[i] = 6000;
    } else {
      samples[i] = -6000;
    }
  }
}


auto Tone::to_raylib() -> Sound {
  Wave wave = {.frameCount = Tone::sample_count,
               .sampleRate = Tone::sample_rate,
               .sampleSize = 16,
               .channels = 1,
               .data = samples,

  };

  return ::LoadSoundFromWave(wave);
}


