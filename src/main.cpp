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

enum class ChipKey : u8 {
  One,
  Two,
  Three,
  C,
  Four,
  Five,
  Six,
  D,
  Seven,
  Eight,
  Nine,
  E,
  A,
  Zero,
  B,
  F
};

constexpr int instructions_per_frame = 400;

auto main(void) -> int {
  State state{};
  state.load_rom(std::cin);

  constexpr int block_size = 16; // in screen pixels

  SetTargetFPS(60);
  InitWindow(Chip8Width * block_size, Chip8Height * block_size, "Chip8");


  while (!WindowShouldClose()) {
    if (state.DT)
      --state.DT;
    if (state.ST)
      --state.ST;
    auto instruction = state.instruction_at(state.PC);

    BeginDrawing();
    EndDrawing();
  }
  CloseWindow();

  return 0;
}

