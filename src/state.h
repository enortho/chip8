#pragma once
#include <array>
#include <cstdint>
#include <istream>
#include <random>
#include <unordered_map>
#include <raylib.h>
#include "instruction.h"


using u8 = std::uint8_t;
using u16 = std::uint16_t;
constexpr u16 Chip8MaxAddress = 0xFFF;
constexpr u16 Chip8ProgramStart = 0x200;
constexpr int Chip8Width = 64;
constexpr int Chip8Height = 32;
constexpr int Chip8StackSize = 16;
constexpr int Chip8MemorySize = 4096;

struct State {
  int screen_width{}, screen_height{};
  int pixel_size{16}; // size in pixels
  std::array<u8, 16> v_registers{};  // V0, V1, ..., VF
  u16 I{}; // stores addresses, so only up to 0xfff / 12 bits
  u16 PC{Chip8ProgramStart}; // program counter
  u8 SP{};  // stack pointer (top of stack)
  u8 DT{};  // delay timer
  u8 ST{};  // sound timer

  std::array<u16, Chip8StackSize> stack{};

  std::array<u8, Chip8MemorySize> memory{};
  std::array<bool, 16> keyboard{};

  static const u16 DIGIT_SPRITES_START_ADDRESS{0x0A0};
  using digit_sprite = std::array<u8, 5>;  // each byte is a row
  std::array<bool, Chip8Width * Chip8Height> screen{};

  static const u8 NOT_WAITING = 0x10;
  u8 waiting_for_key_press{NOT_WAITING}; // the register (0-F) to put the value of the next keypress in, or NOT_WAITING

  std::mt19937 random_alg;
  std::uniform_int_distribution<int> random_generator{0x0, 0xFF};


  ::Sound sound;
  State(int pixel_size);
  ~State();

  auto load_rom(std::istream &stream) -> void;

  auto step() -> bool;
  auto instruction_at(u16 address) -> std::unique_ptr<Instruction>;
  auto poll_input() -> void;
  auto draw() -> void;
  auto load_digit_sprites() -> void;
};
