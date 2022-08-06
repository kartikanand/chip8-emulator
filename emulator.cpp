#include <cassert>
#include <iostream>
#include <string>

#include "emulator.h"

namespace {
// nibble seq should go from 1 to 4
INSTR get_nibble(const INSTR dword, int nibble_seq) {
  assert(nibble_seq >= 1);
  assert(nibble_seq <= 4);

  const int shift = 4 - nibble_seq;
  const int shift_by = 4 * shift;

  return (dword & (0xf << shift_by)) >> shift_by;
}
};  // namespace

Emulator::Emulator() {
  display_ = std::make_unique<Display>(64, 32);
}

INSTR Emulator::fetch() {
  // Combine two ram bytes to form a single 16 bit instruction
  const INSTR current_instruction = ((ram_[PC_] << 8) | (ram_[PC_ + 1]));

  // Point to next instruction
  PC_ += 2;

  return current_instruction;
}

void Emulator::decode_and_execute(const INSTR current_instruction) {
  const INSTR K = get_nibble(current_instruction, 1);
  const INSTR X = get_nibble(current_instruction, 2);
  const INSTR Y = get_nibble(current_instruction, 3);
  const INSTR N = get_nibble(current_instruction, 4);

  const int VX = var_registers_[X];
  const int VY = var_registers_[Y];

  const INSTR NN = (Y << 8) ^ N;
  const INSTR NNN = (X << 16) ^ NN;

  switch (K) {
    case 0: {
      switch (NNN) {
        case 0x0E0:
          display_->clear();
          break;
        case 0x0EE:
          PC_ = stack_.top();
          stack_.pop();
          break;
        default:
          break;
      }
    } break;
    case 1:
      PC_ = NNN;
      break;
    case 2:
      stack_.push(PC_);
      PC_ = NNN;
      break;
    case 3:
      if (VX == NN) {
        PC_ += 1;
      }
      break;
    case 4:
      if (VX != NN) {
        PC_ += 1;
      }
      break;
    case 5:
      if (VX == VY) {
        PC_ += 1;
      }
      break;
    case 6:
      var_registers_[X] = NN;
      break;
    case 7:
      handleAdd(X, var_registers_[Y]);
      break;
    case 8:
      handleArithmetic(X, Y, N, NN, NNN);
      break;
    case 9:
      if (VX != VY) {
        PC_ += 1;
      }
      break;
    case 0xA:
      I_ = NNN;
      break;
    case 0xB:
      PC_ = (var_registers_[0] + NNN) % 255;
      break;
    case 0xC:
      // TODO: random number generator
      var_registers_[X] = NN;
    case 0xD:
      // TODO: display
      for (int i = 0; i < N; ++i) {
        display_->draw(VX, VY, ram_[I_ + i]);
      }
      break;
    case 0xE:
      // TODO: skip if key input
      break;
    case 0xF:
      switch (NN) {
        case 0x07:
          var_registers_[X] = delay_timer_;
          break;
        case 0x15:
          delay_timer_ = VX;
          break;
        case 0x18:
          sound_timer_ = VX;
          break;
        case 0x1E: {
          bool is_overflow = (I_ + VX) >= 256;
          I_ = (I_ + VX) % 256;
          if (is_overflow) {
            var_registers_[0xF] = 1;
          }
        } break;
        case 0x0A:
          // TODO: handle get input
          break;
        case 0x29:
          // TODO: handle font character
          break;
        case 0x33:
          // TODO: handle binary coded decimal conversion
          break;
        case 0x55:
        case 0x65:
          // TODO: handle memory load and store
          break;
        default:
          break;
      }
    default:
      break;
  };
}

void Emulator::handleArithmetic(const INSTR X,
                                const INSTR Y,
                                const INSTR N,
                                const INSTR NN,
                                const INSTR NNN) {
  switch (N) {
    case 0:
      var_registers_[X] = var_registers_[Y];
      break;
    case 1:
      var_registers_[X] |= var_registers_[Y];
      break;
    case 2:
      var_registers_[X] &= var_registers_[Y];
      break;
    case 3:
      var_registers_[X] ^= var_registers_[Y];
      break;
    case 4:
      handleAdd(X, var_registers_[Y]);
      break;
    case 5:
    case 7:
      handleSubtract(X, Y, N);
      break;
    case 6:
    case 0xE:
      handleShift(X, N);
      break;
    default:
      std::cout << "Error in parsing on " << __FILE__ << " : " << __LINE__
                << std::endl;
  }
}

void Emulator::handleAdd(const INSTR X, const INSTR NN) {
  const int a_sum = (var_registers_[X] + NN);

  const bool is_carry = a_sum > 255;
  var_registers_[X] = (a_sum) % 255;
  if (is_carry) {
    VF_ = 1;
  }
}

void Emulator::handleSubtract(const INSTR X, const INSTR Y, const INSTR N) {
  VF_ = 1;

  const INSTR NN = 255 - (N == 0x5 ? var_registers_[Y] : var_registers_[X]);
  const INSTR a_sum = var_registers_[X] + NN;
  const INSTR a_sub = (a_sum) % 255;

  // borrow happens when there is no carry
  // 4 - 3 (no borrow leads to a carry)
  // 4 + (255 - 3) => (256)%255 = 1
  //
  // 3 - 4 (borrow doesn't lead to a carry)
  // 3 + (255 - 4) => (254)%255 = 254
  const bool is_borrow = a_sub <= 255;
  if (is_borrow) {
    VF_ = 0;
  }

  var_registers_[X] = a_sub;
}

void Emulator::handleShift(const INSTR X, const INSTR N) {
  const INSTR shift = (N == 0x6) ? X >> 1 : X << 1;
  const bool shifted_bit =
      (N == 0x6)
          ? (X & 0x01) == 1    /* check if first bit is set */
          : (X & 0x80) == 0x80 /* check if ;ast bit is set, 10000000 -> 1*0
                                + 2*0 + 4*0 + 8*1 => 80 */
      ;

  VF_ = shifted_bit ? 1 : 0;
}
