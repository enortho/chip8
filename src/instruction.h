#pragma once

#include <memory>
#include <variant>
#include <cstddef>

using u16 = std::uint16_t;


struct State;
struct Instruction {
  struct Next {};
  struct Next2 {};
  struct JumpTo {
    u16 address;
    JumpTo(u16 address) : address{address} {}
  };
  using PcAction = std::variant<Next, Next2, JumpTo>;
  
  auto virtual show() const -> std::string = 0;
  auto virtual act(State &state) -> PcAction = 0;
  virtual ~Instruction() {};
};


auto parse(u16 raw) -> std::unique_ptr<Instruction>;
