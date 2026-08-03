#include <cassert>
#include <cstdint>
#include <iostream>
#include <raylib.h>
#include <cstddef>
#include "state.h"

#include "instruction.h"
// details from http://devernay.free.fr/hacks/chip8/C8TECH10.HTM
using u8 = std::uint8_t;
using u16 = std::uint16_t;


auto main(void) -> int {
  const int pixel_size = 16;
  State state{pixel_size, 440.0f};
  state.load_rom(std::cin);
  
  while (!::WindowShouldClose()) {
    state.poll_input();
    if (state.DT)
      --state.DT;
    if (state.ST) {
      if (!::IsSoundPlaying(state.sound)) {
        ::PlaySound(state.sound);
      }
      --state.ST;
    }

    bool instruction_decode_success = true;
    for (int i = 0; i < state.instructions_per_frame && instruction_decode_success && state.waiting_for_key_press == State::NOT_WAITING; ++i) {
      instruction_decode_success = state.step();
    }

    if (state.waiting_for_key_press == State::NOT_WAITING) {
      ::BeginDrawing();
      state.draw();
      ::EndDrawing();
    }
  }

  return 0;
}

