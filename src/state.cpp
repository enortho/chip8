#include "state.h"
#include <cassert>
#include <print>

auto State::load_rom(std::istream &stream) -> void {
  char buffer;
  int i{};
  while (stream.read(&buffer, 1) && i + Chip8ProgramStart <= Chip8MaxAddress) {
    u8 byte = static_cast<u8>(buffer);
    memory.at(i + Chip8ProgramStart) = byte;
    ++i;
  }

  if (i + Chip8ProgramStart > Chip8MaxAddress) {
    std::println("WARN: ROM is too big, truncating some bytes...");
  }
}


auto State::instruction_at(u16 address) -> std::unique_ptr<Instruction> {
  assert(address <= Chip8MaxAddress && address + 1 <= Chip8MaxAddress);
  assert(address % 2 == 0); // instructions always start at even addresses

  // instructions are big-endian in memory
  u8 hi = memory.at(address);
  u8 lo = memory.at(address + 1);
  u16 raw = (static_cast<u16>(hi) << 8) | static_cast<u16>(lo);
  return parse(raw);
}


auto State::step() -> void {
  std::unique_ptr<Instruction> current_instruction = instruction_at(PC);
  assert(current_instruction != nullptr); // make sure it decoded correctly
  current_instruction->act(*this);
}

