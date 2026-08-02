#include <array>
#include <bit>
#include <cassert>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <istream>
#include <ranges>
#include <print>
#include <raylib.h>
#include <cstddef>
#include <vector>
#include "state.h"
#include "instruction.h"
// details from http://devernay.free.fr/hacks/chip8/C8TECH10.HTM
using u8 = std::uint8_t;
using u16 = std::uint16_t;


constexpr int instructions_per_frame = 100;

auto main(void) -> int {
  const int pixel_size = 16;
  State state{pixel_size};
  state.load_rom(std::cin);

  int i = instructions_per_frame;
  while (i --> 0) {
    state.step();
  }

  auto hi = 0x1319;

  SetTargetFPS(60);
  InitWindow(state.screen_width, state.screen_height, "Chip8");


  while (!WindowShouldClose()) {
    state.poll_input();
    if (state.DT)
      --state.DT;
    if (state.ST)
      --state.ST;

    bool instruction_decode_success = true;
    for (int i = 0; i < instructions_per_frame && instruction_decode_success && state.waiting_for_key_press == State::NOT_WAITING; ++i) {
      instruction_decode_success = state.step();
    }

    if (state.waiting_for_key_press == State::NOT_WAITING) {
      BeginDrawing();
      state.draw();
      EndDrawing();
    }
  }
  CloseWindow();

  return 0;
}

