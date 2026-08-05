#include <cassert>
#include <cstdint>
#include <iostream>
#include <raylib.h>
#include <cstddef>
#include "state.h"
#include "instruction.h"

using u8 = std::uint8_t;
using u16 = std::uint16_t;

#if defined(PLATFORM_WEB)
#include <fstream>
#include "ffi.h"
#endif

// details from http://devernay.free.fr/hacks/chip8/C8TECH10.HTM


void UpdateDrawFrame(State &state);
auto EmscriptenLoop(void *arg) -> void {
  UpdateDrawFrame(*static_cast<State*>(arg));
}


auto main(void) -> int {
  const int pixel_size = 16;
  State state{pixel_size, 440.0f};

  // for desktop, read in roms via stdin.
  // eg. ./chip8 < pong.ch8 to run the pong program
#if defined(PLATFORM_DESKTOP)
  state.load_rom(std::cin);
#else
  // for web, start with pong as the program
  // users can load in other roms with the ffi in "ffi.h"
  global_state = &state;
  std::ifstream rom("/roms/PONG2", std::ios::binary);
  state.load_rom(rom);
#endif


  
#if defined(PLATFORM_WEB)
  emscripten_set_main_loop_arg(EmscriptenLoop, &state, 0, 1);
#else
  while (!::WindowShouldClose()) {
    UpdateDrawFrame(state);
  }
#endif
  
  return 0;
}


auto UpdateDrawFrame(State &state) -> void {
  // in emscripten, requestAnimationFrame is used to power the loop, so FPS may
  // not be close to 60. Count the time manually for this to ensure the sound
  // and delay timers are updated at 60hz
  static double time_since_last_update = 0.0;
  constexpr double time_per_update = 1.0 / 60.0;

  state.poll_input();

  time_since_last_update += ::GetFrameTime();
  while (time_since_last_update >= time_per_update) {
    if (state.DT)
      --state.DT;
    if (state.ST) {
      if (!::IsSoundPlaying(state.sound)) {
        ::PlaySound(state.sound);
      }
      --state.ST;
    }
    time_since_last_update -= time_per_update;
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

