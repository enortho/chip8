#pragma once
#include <array>
#include <cstdint>
#include <istream>
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
  std::array<u8, 16> v_registers{};  // V0, V1, ..., VF
  u16 I{}; // stores addresses, so only up to 0xfff / 12 bits
  u16 PC{Chip8ProgramStart}; // program counter
  u8 SP{};  // stack pointer (top of stack)
  u8 DT{};  // delay timer
  u8 ST{};  // sound timer
  
  std::array<u16, Chip8StackSize> stack{};
  std::array<u8, Chip8MemorySize> memory{};
  std::array<bool, 16> keys{};

  std::array<u8, Chip8Width * Chip8Height> screen{};
  bool waiting_for_key_press{};

  auto load_rom(std::istream &stream) -> void;

  auto step() -> void;
  auto instruction_at(u16 address) -> std::unique_ptr<Instruction>;
};
