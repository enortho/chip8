#include <cassert>
#include <memory>
#include <string>
#include <print>
#include <vector>
#include "instruction.h"

#include "state.h"

using u8 = std::uint8_t;
using u16 = std::uint16_t;
using i16 = std::int16_t;


auto addr_in_range(u16 chip8_address) -> bool {
  return chip8_address < Chip8MaxAddress;
}
template <typename T>
auto addr_in_range(T chip8_address) -> bool = delete;


struct Cls : Instruction {
  auto virtual show() const -> std::string override { return "00E0 - CLS"; }
  auto virtual act(State &state) -> PcAction override {
    state.screen.fill(0);
    return Next{};
  }
};

struct Ret : Instruction {
  auto show() const -> std::string override { return "0EEE - RET"; }
  auto act(State &state) -> PcAction override {
    auto return_address = state.stack.at(state.SP);
    if (state.SP > 0) {
      --state.SP;
    }
    return JumpTo{return_address};
  }
};

struct Jp : Instruction {
  u16 location{}; // location to jump to
  Jp(u16 location) : location{location} {}
  auto show() const -> std::string override { return std::format("1nnn - JP 0x{:X}", location); }
  auto act(State &state) -> PcAction override {
    return JumpTo{location};
  }
};

struct Call : Instruction {
  u16 location{}; // call subroutine (only 12 bits used)
  Call(u16 location) : location{location} {}
  auto show() const -> std::string override {
    return std::format("2nnn - CALL 0x{:X}", location);
  }
  auto act(State &state) -> PcAction override {
    // increment stack pointer, put PC on the stack, set PC to nnn
    assert(state.SP < 16 - 1);
    ++state.SP;
    state.stack.at(state.SP) = state.PC;
    return JumpTo{location};
  }
};

struct SeImm : Instruction {
  u8 v_x{}; // 4 bits only!
  u8 immediate{};
  SeImm(u8 v_x, u8 immediate) : v_x{v_x}, immediate{immediate} {}
  auto show() const-> std::string override { return std::format("3xkk - SE V{:X}, 0x{:X}", v_x, immediate); }
  auto act(State &state) -> PcAction override {
    // skip next instruction if Vc = kk
    if (state.v_registers.at(v_x) == immediate) {
      return Next2{};
    }
    return Next{};
  }
};

struct SneImm : Instruction {
  u8 v_x{};
  u8 immediate{};
  SneImm(u8 v_x, u8 immediate) : v_x{v_x}, immediate{immediate} {}
  auto show() const -> std::string override {
    return std::format("4xkk - SNE V{:X}, 0x{:X}", v_x, immediate);
  }
  auto act(State &state) -> PcAction override {
    // skip next instruction if Vx != kk
    if (state.v_registers.at(v_x) != immediate) {
      return Next2{};
    }
    return Next{};
  }
};


struct Se : Instruction {
  u8 v_x{};
  u8 v_y{};

  Se(u8 v_x, u8 v_y) : v_x{v_x}, v_y{v_y} {}
  auto show() const -> std::string override {
    return std::format("5xy0 - SE V{:X}, V{:X}", v_y, v_y);
  }
  
  auto act(State &state) -> PcAction override {
    // skip next instruction if Vx == Vy
    if (state.v_registers.at(v_x) !=
        state.v_registers.at(v_y)) {
      return Next2{};
    }
    return Next{};
  }
};


struct LdImm : Instruction {
  u8 v_x{};
  u8 immediate{};
  LdImm(u8 v_x, u8 immediate) : v_x{v_x}, immediate{immediate} {}
  auto show() const -> std::string override {
    return std::format("6xkk - LD V{:X}, 0x{:X}", v_x, immediate);
  }
  auto act(State &state) -> PcAction override {
    state.v_registers.at(v_x) = immediate;
    return Next{};
  }
};

struct AddImm : Instruction {
  u8 v_x{};
  u8 immediate{};
  AddImm(u8 v_x, u8 immediate) : v_x{v_x}, immediate{immediate} {}
  auto show() const -> std::string override {
    return std::format("7xkk - ADD V{:X}, 0x{:X}", v_x, immediate);
  }
  auto act(State &state) -> PcAction override {
    state.v_registers.at(v_x) += immediate;
    return Next{};
  }
};

struct Ld : Instruction {
  u8 v_x{};
  u8 v_y{};
  Ld(u8 v_x, u8 v_y) : v_x{v_x}, v_y{v_y} {}
  auto show() const -> std::string override {
    return std::format("8xy0 - LD V{:X}, V{:X}", v_x, v_y);
  }
  auto act(State &state) -> PcAction override {
    state.v_registers.at(v_x) = state.v_registers.at(v_y);
    return Next{};
  }
};

struct Or : Instruction {
  u8 v_x{};
  u8 v_y{};
  Or(u8 v_x, u8 v_y) : v_x{v_x}, v_y{v_y} {}
  auto show() const -> std::string override {
    return std::format("8xy1 - OR V{:X}, V{:X}", v_x, v_y);
  }

  auto act(State &state) -> PcAction override {
    state.v_registers.at(v_x) =
        state.v_registers.at(v_x) | state.v_registers.at(v_y);
    return Next{};
  }
};


struct And : Instruction {
  u8 v_x{};
  u8 v_y{};
  And(u8 v_x, u8 v_y) : v_x{v_x}, v_y{v_y} {}
  auto show() const -> std::string override {
    return std::format("8xy2 - AND V{:X}, V{:X}", v_x, v_y);
  }

  auto act(State &state) -> PcAction override {
    state.v_registers.at(v_x) =
        state.v_registers.at(v_x) & state.v_registers.at(v_y);
    return Next{};
  }
};

struct Xor : Instruction {
  u8 v_x{};
  u8 v_y{};
  Xor(u8 v_x, u8 v_y) : v_x{v_x}, v_y{v_y} {}
  auto show() const -> std::string override {
    return std::format("8xy3 - XOR V{:X}, V{:X}", v_x, v_y);
  }

  auto act(State &state) -> PcAction override {
    state.v_registers.at(v_x) =
        state.v_registers.at(v_x) ^ state.v_registers.at(v_y);
    return Next{};
  }
};

struct Add : Instruction {
  u8 v_x{};
  u8 v_y{};
  Add(u8 v_x, u8 v_y) : v_x{v_x}, v_y{v_y} {}
  auto show() const -> std::string override {
    return std::format("8xy4 - ADD V{:X}, V{:X}", v_x, v_y);
  }

  auto act(State &state) -> PcAction override {
    u16 result = (u16)state.v_registers.at(v_x) +
      (u16)state.v_registers.at(v_y);
    bool has_carry = result > 255;
    u8 truncated_result = (u8)result;
    
    state.v_registers.at(v_x) = truncated_result;

    u8 carry = has_carry ? 1 : 0;
    state.v_registers[0xF] = carry;
    return Next{};
  }
};

struct Sub : Instruction {
  u8 v_x{};
  u8 v_y{};
  Sub(u8 v_x, u8 v_y) : v_x{v_x}, v_y{v_y} {}
  auto show() const -> std::string override {
    return std::format("8xy5 - SUB V{:X}, V{:X}", v_x, v_y);
  }

  auto act(State &state) -> PcAction override {
    auto v_first = (i16)state.v_registers.at(v_x);
    auto v_second = (i16)state.v_registers.at(v_y);
    i16 result = v_first - v_second;

    state.v_registers[v_x] = (u8)(v_first - v_second);

    if (v_first > v_second) {
      state.v_registers[0xF] = 1;
    } else {
      state.v_registers[0xF] = 0;
    }

    return Next{};
  }
};


struct Shr : Instruction {
  u8 v_x{};
  Shr(u8 v_x) : v_x{v_x} {}
  auto show() const -> std::string override {
    return std::format("8xy6 - SHR V{:X}", v_x);
  }
  auto act(State &state) -> PcAction override {
    bool least_significant_bit_of_vx_is_1 = v_x & 1;
    if (least_significant_bit_of_vx_is_1) {
      state.v_registers[0xF] = 1;
    } else {
      state.v_registers[0xF] = 0;
    }

    state.v_registers.at(v_x) /= 2;
    return Next{};
  }
};

struct Subn : Instruction {
  u8 v_x{};
  u8 v_y{};
  Subn(u8 v_x, u8 v_y) : v_x{v_x}, v_y{v_y} {}
  auto show() const -> std::string override {
    return std::format("8xy7 - SUBN V{:X}, V{:X}", v_x, v_y);
  }

  auto act(State &state) -> PcAction override {
    auto v_first = (i16)state.v_registers.at(v_x);
    auto v_second = (i16)state.v_registers.at(v_y);
    
    state.v_registers[v_x] = (u8)(v_second - v_first);

    if (v_second > v_first) {
      state.v_registers[0xF] = 1;
    } else {
      state.v_registers[0xF] = 0;
    }

    return Next{};
  }
};


struct Shl : Instruction {
  u8 v_x{};
  Shl(u8 v_x) : v_x{v_x}{}
  auto show() const -> std::string override {
    return std::format("8xyE - SHL V{:X}", v_x);
  }

  auto act(State &state) -> PcAction override {
    bool most_significant_bit_of_vx_is_1 = state.v_registers.at(v_x) & (1 << 7);
    if (most_significant_bit_of_vx_is_1) {
      state.v_registers[0xF] = 1;
    } else {
      state.v_registers[0xF] = 0;
    }
    return Next{};
  }
};

struct Sne : Instruction {
  u8 v_x{};
  u8 v_y{};
  Sne(u8 v_x, u8 v_y) : v_x{v_x}, v_y{v_y} {}
  auto show() const -> std::string override {
    return std::format("9xy0 - SNE V{:X}, V{:X}", v_x, v_y);
  }

  auto act(State &state) -> PcAction override {
    if (state.v_registers[v_x] != state.v_registers[v_y]) {
      return Next2{};
    }
    return Next{};
  }
};

struct LdI : Instruction {
  u16 addr{};
  LdI(u16 addr) : addr{addr} {}
  auto show() const -> std::string override {
    return std::format("Annn - LD I, 0x{:X}", addr);
  }

  auto act(State &state) -> PcAction override {
    state.I = addr;
    return Next{};
  }
};

struct JpV0 : Instruction {
  u16 addr{};
  JpV0(u16 addr) : addr{addr} {}
  auto show() const -> std::string override {
    return std::format("Bnnn - JP V0, 0x{:X}", addr);
  }

  auto act(State &state) -> PcAction override {
    u16 address = static_cast<u16>(state.v_registers[0x0]) + addr;
    return JumpTo{address};
  }
};


struct Rnd : Instruction {
  u8 v_x{};
  u8 immediate{};
  Rnd(u8 v_x, u8 immediate) : v_x{v_x}, immediate{immediate} {}
  auto show() const -> std::string override {
    return std::format("Cxkk - RND V{:X}, 0x{:X}", v_x, immediate);
  }

  auto act(State &state) -> PcAction override {
    u8 random_number = 4; // chosen by a fair, random dice roll
                          // guaranteed to be fair
    state.v_registers.at(v_x) = random_number & immediate;
    return Next{};
  }
};

struct Drw : Instruction {
  u8 v_x{};
  u8 v_y{};
  u8 n{};

  Drw(u8 v_x, u8 v_y, u8 n) : v_x{v_x}, v_y{v_y}, n{n} {}
  auto show() const -> std::string override {
    return std::format("Dxyn - DRW V{:X}, V{:X}, 0x{:X}", v_x, v_y, n);
  }


  auto act(State &state) -> PcAction override {
    bool some_pixel_erased = false;

    /*
      (0,0) ... (63, 0)

      (0,31) .. (63, 31)
      so it's like (col, row)
      to flatten out: col + row*64
     */
    auto start_col = state.v_registers.at(v_x) % Chip8Width;
    auto start_row = state.v_registers.at(v_y) % Chip8Height;

    for (int byte_i = 0; byte_i < n; ++byte_i) {
      u8 byte = state.memory.at(state.I + byte_i);
      u8 row_to_draw_this_byte =
        (start_row + byte_i) % Chip8Height; // each byte on a different row

      constexpr int bits_per_byte = 8;
      for (int bit_index = 0; bit_index < bits_per_byte; ++bit_index) {
        auto col_to_draw_at = (start_col + bit_index) % Chip8Width;
        auto byte_index_to_draw_at = row_to_draw_this_byte * Chip8Width +
                                     (col_to_draw_at / bits_per_byte);
        u8 thing_to_xor_in = 1 << (bits_per_byte - bit_index);

        u8 &current_byte_in_memory = state.memory.at(byte_index_to_draw_at);
        u8 changed_byte_in_memory = thing_to_xor_in ^ current_byte_in_memory;

        some_pixel_erased = some_pixel_erased ||
                            (changed_byte_in_memory < current_byte_in_memory);

        current_byte_in_memory = changed_byte_in_memory;        
      }
    }
    


    if (some_pixel_erased) {
      state.v_registers[0xF] = 1;
    } else {
      state.v_registers[0xF] = 0;
    }
    return Next{};
  }
};


struct Skp : Instruction {
  u8 v_x{};
  Skp(u8 v_x) : v_x{v_x} {}
  auto show() const -> std::string override {
    return std::format("Ex9E - SKP V{:X}", v_x);
  }

  auto act(State &state) -> PcAction override {
    auto register_value = state.v_registers.at(v_x);
    auto key_pressed = state.keyboard.at(register_value);
    if (key_pressed) {
      return Next2{};
    }
    return Next{};
  }
};


struct Sknp : Instruction {
  u8 v_x{};
  Sknp(u8 v_x) : v_x{v_x} {}
  auto show() const -> std::string override {
    return std::format("ExA1 - SKNP V{:X}", v_x);
  }

  auto act(State &state) -> PcAction override {
    auto register_value = state.v_registers.at(v_x);
    auto key_pressed = state.keyboard.at(register_value);
    if (!key_pressed) {
      return Next2{};
    }
    return Next{};
  }
};


struct LdVxDT : Instruction {
  u8 v_x{};
  LdVxDT(u8 v_x) : v_x{v_x} {}
  auto show() const -> std::string override {
    return std::format("Fx07 - LD V{:X}, DT", v_x);
  }

  auto act(State &state) -> PcAction override {
    state.v_registers[v_x] = state.DT;
    return Next{};
  }
};


struct LdKey : Instruction {
  u8 v_x{};
  LdKey(u8 v_x) : v_x{v_x} {}
  auto show() const -> std::string override {
    return std::format("Fx0A - LD V{:X}, K", v_x);
  }

  auto act(State &state) -> PcAction override {
    state.waiting_for_key_press = v_x;
    return Next{};
  }
};


struct LdDTVx : Instruction {
  u8 v_x{};
  LdDTVx(u8 v_x) : v_x{v_x} {}
  auto show() const -> std::string override {
    return std::format("Fx15 - LD DT, V{:X}", v_x);
  }

  auto act(State &state) -> PcAction override {
    state.ST = state.v_registers.at(v_x);
    return Next{};
  }
};


struct LdSt : Instruction {
  u8 v_x{};
  LdSt(u8 v_x) : v_x{v_x} {}
  auto show() const -> std::string override {
    return std::format("Fx18 - LD ST, V{:X}", v_x);
  }

  auto act(State &state) -> PcAction override {
    state.ST = state.v_registers.at(v_x);
    return Next{};
  }
};

struct AddI : Instruction {
  u8 v_x{};
  AddI(u8 v_x) : v_x{v_x} {}
  auto show() const -> std::string override {
    return std::format("Fx1E - ADD I, V{:X}", v_x);
  }

  auto act(State &state) -> PcAction override {
    state.I += state.v_registers.at(v_x);
    return Next{};
  }
};

struct LdF : Instruction {
  u8 v_x{};
  LdF(u8 v_x) : v_x{v_x} {}
  auto show() const -> std::string override {
    return std::format("Fx29 - LD F, V{:X}", v_x);
  }

  auto act(State &state) -> PcAction override {
    constexpr int digit_size = sizeof(State::digit_sprite);
    assert(v_x <= 0xF);
    state.I = State::DIGIT_SPRITES_START_ADDRESS + v_x * digit_size;
    assert(state.I < Chip8MaxAddress);
    return Next{};
  }
};

struct LdB : Instruction {
  u8 v_x{};
  LdB(u8 v_x) : v_x{v_x} {}
  auto show() const -> std::string override {
    return std::format("Fx33 - LD B, V{:X}", v_x);
  }

  auto act(State &state) -> PcAction override {
    auto v_x_val = state.v_registers.at(v_x);
    u8 hundreds = (v_x_val / 100) % 10;
    u8 tens = (v_x_val / 10) % 10;
    u8 ones = v_x_val % 10;

    state.memory.at(state.I) = hundreds;
    state.memory.at(state.I + 1) = tens;
    state.memory.at(state.I + 2) = ones;
    return Next{};
  }
};


struct LdIV : Instruction {
  u8 v_x{};
  LdIV(u8 v_x) : v_x{v_x} {}
  auto show() const -> std::string override {
    return std::format("Fx55 - LD [I], V{:X}", v_x);
  }

  auto act(State &state) -> PcAction override {
    // load V0 through Vx into memory starting at location I
    for (int i{}; i < v_x; ++i) {
      u16 address_to_set = state.I + i;
      assert(addr_in_range(address_to_set));
      state.memory.at(address_to_set) = state.v_registers.at(i);
    }

    return Next{};
  }
};


struct LdVI : Instruction {
  u8 v_x{};
  LdVI(u8 v_x) : v_x{v_x} {}
  auto show() const -> std::string override {
    return std::format("Fx65 - LD V{:X}, [I]", v_x);
  }

  auto act(State &state) -> PcAction override {
    for (int i{}; i < v_x; ++i) {
      u16 address_to_read = state.I + i;
      assert(addr_in_range(address_to_read));
      state.v_registers.at(i) = state.memory.at(address_to_read);
    }

    return Next{};
  }
};



auto parse(u16 raw) -> std::unique_ptr<Instruction> {
  using std::make_unique;
  u8 lo = (u8)raw;
  u8 l1 = lo & 0x0F;
  u8 l2 = (lo & 0xF0) >> 4;
  u8 hi = (u8)(raw >> 8);
  u8 h1 = hi & 0x0F;
  u8 h2 = (hi & 0xF0) >> 4;

  u16 nnn = raw & 0x0FFF;
  u8 x = h1;
  u8 y = l2;
  u8 kk = lo;

  if (raw == 0x00E0) {
    return make_unique<Cls>();
  } else if (raw == 0x00EE) {
    return make_unique<Ret>();
  }

  
  switch (h2) {
  case 0x1:
    return make_unique<Jp>(nnn);
  case 0x2:
    return make_unique<Call>(nnn);
  case 0x3:
    return make_unique<SeImm>(x, kk);
  case 0x4:
    return make_unique<SneImm>(x, kk);
  case 0x5:
    return make_unique<Se>(x, y);
  case 0x6:
    return make_unique<LdImm>(x, kk);
  case 0x7:
    return make_unique<AddImm>(x, kk);
  case 0x8: {
    switch (l1) {
    case 0:
      return make_unique<Ld>(x, y);
    case 1:
      return make_unique<Or>(x, y);
    case 2:
      return make_unique<And>(x, y);
    case 3:
      return make_unique<Xor>(x, y);
    case 4:
      return make_unique<Add>(x, y);
    case 5:
      return make_unique<Sub>(x, y);
    case 6:
      return make_unique<Shr>(x);
    case 7:
      return make_unique<Subn>(x, y);
    case 0xE:
      return make_unique<Shl>(x);
    default:
      return nullptr;
    }

  }
  case 0x9: {
    if (l1 != 0) {
      std::println("WARNING: Instruction is 9xy{:X} but should be 9xy0", l1);
    }
    return make_unique<Sne>(x, y);
  }
  case 0xA:
    return make_unique<LdI>(nnn);
  case 0xB:
    return make_unique<JpV0>(nnn);
  case 0xC:
    return make_unique<Rnd>(x, kk);
  case 0xD:
    return make_unique<Drw>(x, y, l1);
  case 0xE: {
    switch (lo) {
    case 0x9E:
      return make_unique<Skp>(x);
    case 0xA1:
      return make_unique<Sknp>(x);
    default: return nullptr;
    }
  }
  case 0xF: {
    switch (lo) {
    case 0x07:
      return make_unique<LdVxDT>(x);
    case 0x0A:
      return make_unique<LdKey>(x);
    case 0x15:
      return make_unique<LdDTVx>(x);
    case 0x18:
      return make_unique<LdSt>(x);
    case 0x1E:
      return make_unique<AddI>(x);
    case 0x29:
      return make_unique<LdF>(x);
    case 0x33:
      return make_unique<LdB>(x);
    case 0x55:
      return make_unique<LdIV>(x);
    case 0x65:
      return make_unique<LdVI>(x);
    default: return nullptr;
    }
  }
  }

  return nullptr;
}

