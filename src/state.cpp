#include "state.h"
#include "overloaded.h"
#include <raylib.h>
#include <cassert>
#include <print>
#include <variant>

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

  // instructions are big-endian in memory
  u8 hi = memory.at(address);
  u8 lo = memory.at(address + 1);
  u16 raw = (static_cast<u16>(hi) << 8) | static_cast<u16>(lo);
  return parse(raw);
}


auto State::step() -> bool {
  std::unique_ptr<Instruction> current_instruction = instruction_at(PC);
  bool success = current_instruction != nullptr;
  if (!success) {
    return false;
  }
  assert(current_instruction != nullptr); // make sure it decoded correctly

  std::println("0x{:X} | {}", PC, current_instruction->show());
  // execute the instruction
  Instruction::PcAction pc_action = current_instruction->act(*this);

  // go to next instruction, skip the next instruciton, or jump to a different
  // instruction entirely
  // idk why it formats like this:(
  std::visit(overloaded{[&](const Instruction::Next) { PC += 2; },
                        [&](const Instruction::Next2) { PC += 4; },
                        [&](const Instruction::JumpTo &jump_action) {
                          PC = jump_action.address;
                        }},
             pc_action);
  return true;
}

auto State::poll_input() -> void {
  // from https://github.com/thehackersbrain/chip8
  /*
    CHIP-8   Keyboard
    -----------------
    1 2 3 C   1 2 3 4
    4 5 6 D   Q W E R
    7 8 9 E   A S D F
    A 0 B F   Z X C V
   */
  const auto k0 = KEY_X;
  const auto k1 = KEY_ONE;
  const auto k2 = KEY_TWO;
  const auto k3 = KEY_THREE;
  const auto k4 = KEY_Q;
  const auto k5 = KEY_W;
  const auto k6 = KEY_E;
  const auto k7 = KEY_A;
  const auto k8 = KEY_S;
  const auto k9 = KEY_D;
  const auto ka = KEY_Z;
  const auto kb = KEY_C;
  const auto kc = KEY_FOUR;
  const auto kd = KEY_R;
  const auto ke = KEY_F;
  const auto kf = KEY_V;
  std::array<KeyboardKey, 16> key_map = {k0, k1, k2, k3, k4, k5, k6, k7,
                                         k8, k9, ka, kb, kc, kd, ke, kf};
  for (int i = 0; const auto &key : key_map) {
    auto key_down = ::IsKeyDown(key);
    this->keyboard.at(i) = key_down;
    if (key_down && waiting_for_key_press != NOT_WAITING) {
      v_registers.at(waiting_for_key_press) = key;
      waiting_for_key_press = NOT_WAITING;
    }
  }
  
  
}

auto State::load_digit_sprites() -> void {
  const auto start = State::DIGIT_SPRITES_START_ADDRESS;

  constexpr std::array<digit_sprite, 16> digits = {{
      {0xF0, 0x90, 0x90, 0x90, 0xF0},  // 0
      {0x20, 0x60, 0x20, 0x20, 0x70},  // 1
      {0xF0, 0x10, 0xF0, 0x80, 0xF0},  // 2
      {0xF0, 0x10, 0xF0, 0x10, 0xF0},  // 3
      {0x90, 0x90, 0xF0, 0x10, 0x10},  // 4
      {0xF0, 0x80, 0xF0, 0x10, 0xF0},  // 5
      {0xF0, 0x80, 0xF0, 0x90, 0xF0},  // 6
      {0xF0, 0x10, 0x20, 0x40, 0x40},  // 7
      {0xF0, 0x90, 0xF0, 0x90, 0xF0},  // 8
      {0xF0, 0x90, 0xF0, 0x10, 0xF0},  // 9
      {0xF0, 0x90, 0xF0, 0x90, 0x90},  // A
      {0xE0, 0x90, 0xE0, 0x90, 0xE0},  // B
      {0xF0, 0x80, 0x80, 0x80, 0xF0},  // C
      {0xE0, 0x90, 0x90, 0x90, 0xE0},  // D
      {0xF0, 0x80, 0xF0, 0x80, 0xF0},  // E
      {0xF0, 0x80, 0xF0, 0x80, 0x80},  // F
  }};


  constexpr int digit_size = sizeof(digit_sprite);
  for (int digit_i = 0; const auto &digit : digits) {
    auto offset = digit_size * digit_i + State::DIGIT_SPRITES_START_ADDRESS;
    std::copy(digit.begin(), digit.end(), memory.begin() + offset);
  }
}


auto State::draw() -> void {
  ClearBackground(RAYWHITE);
  for (int i = 0; const auto &byte : screen) {
    auto row_i = i / Chip8Width;
    auto start_col = i % Chip8Height;
    for (int i = 0; i < 8; ++i) {
      auto col = start_col + i;
      u8 mask = 1 << (8 - i);
      auto tile = mask & byte;

      Color color = tile ? RAYWHITE : BLACK;

      auto x = col * pixel_size;
      auto y = col = row_i * pixel_size;
      DrawRectangle(x, y, pixel_size, pixel_size, color);
    }
  }
}

