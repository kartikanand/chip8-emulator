#include <array>
#include <cassert>
#include <fstream>
#include <iostream>
#include <map>
#include <optional>
#include <random>
#include <string>

#include <SFML/Graphics.hpp>

#include "emulator.h"

namespace {
const std::map<INSTR, const std::array<INSTR, 5>> font_bytes_map{
    {0x0, {0xF0, 0x90, 0x90, 0x90, 0xF0}},
    {0x1, {0x20, 0x60, 0x20, 0x20, 0x70}},
    {0x2, {0xF0, 0x10, 0xF0, 0x80, 0xF0}},
    {0x3, {0xF0, 0x10, 0xF0, 0x10, 0xF0}},
    {0x4, {0x90, 0x90, 0xF0, 0x10, 0x10}},
    {0x5, {0xF0, 0x80, 0xF0, 0x10, 0xF0}},
    {0x6, {0xF0, 0x80, 0xF0, 0x90, 0xF0}},
    {0x7, {0xF0, 0x10, 0x20, 0x40, 0x40}},
    {0x8, {0xF0, 0x90, 0xF0, 0x90, 0xF0}},
    {0x9, {0xF0, 0x90, 0xF0, 0x10, 0xF0}},
    {0xA, {0xF0, 0x90, 0xF0, 0x90, 0x90}},
    {0xB, {0xE0, 0x90, 0xE0, 0x90, 0xE0}},
    {0xC, {0xF0, 0x80, 0x80, 0x80, 0xF0}},
    {0xD, {0xE0, 0x90, 0x90, 0x90, 0xE0}},
    {0xE, {0xF0, 0x80, 0xF0, 0x80, 0xF0}},
    {0xF, {0xF0, 0x80, 0xF0, 0x80, 0x80}}};

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
  window_ = std::make_unique<sf::RenderWindow>(
      sf::VideoMode(width_ * scale_, height_ * scale_), "Chip8 Emulator");

  for (int i = 0; i < (width_ * height_); ++i) {
    pixels_.push_back(0);
  }

  reset();
}

void Emulator::load_program(const std::string& file_name) {
  unsigned short program_start_address = program_start_address_;

  std::ifstream file(file_name, std::ios::binary);
  unsigned char buffer;
  file.read(reinterpret_cast<char*>(&buffer), 1);

  while (file) {
    const INSTR opcode = (INSTR)buffer;
    ram_[program_start_address++] = opcode;
    file.read(reinterpret_cast<char*>(&buffer), 1);
  }
}

void Emulator::loop() {
  PC_ = program_start_address_;

  while (window_->isOpen()) {
    sf::Event event;
    while (window_->pollEvent(event)) {
      if (event.type == sf::Event::Closed)
        window_->close();
    }

    INSTR current_instruction = fetch();

    if (current_instruction) {
      // Point to next instruction
      PC_ += 2;

      decode_and_execute(current_instruction);
    }

    window_->clear(sf::Color::White);

    draw_pixels();

    window_->display();
  }
}

void Emulator::reset() {
  // Clear screen
  clear();

  // Zerofi the ram
  std::fill(std::begin(ram_), std::end(ram_), 0x0);

  for (INSTR i = 0; i <= 0xF; ++i) {
    var_registers_[i] = 0;
  }

  // Load font data in ram
  unsigned int i = 0;
  for (auto const& [_, val] : font_bytes_map) {
    for (auto const& font_byte : val) {
      ram_[i++] = font_byte;
    }
  }
}

INSTR Emulator::fetch() {
  // Combine two ram bytes to form a single 16 bit instruction
  const INSTR current_instruction = ((ram_[PC_] << 8) | (ram_[PC_ + 1]));

  return current_instruction;
}

void Emulator::decode_and_execute(const INSTR current_instruction) {
  const INSTR K = get_nibble(current_instruction, 1);
  const INSTR X = get_nibble(current_instruction, 2);
  const INSTR Y = get_nibble(current_instruction, 3);
  const INSTR N = get_nibble(current_instruction, 4);

  const int VX = var_registers_[X];
  const int VY = var_registers_[Y];

  const INSTR NN = (Y << 4) ^ N;
  const INSTR NNN = (X << 8) ^ NN;

  switch (K) {
    case 0: {
      switch (NNN) {
        case 0x0E0:
          clear();
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
        PC_ += 2;
      }
      break;
    case 4:
      if (VX != NN) {
        PC_ += 2;
      }
      break;
    case 5:
      if (VX == VY) {
        PC_ += 2;
      }
      break;
    case 6:
      var_registers_[X] = NN;
      break;
    case 7:
      handleAdd(X, NN);
      break;
    case 8:
      handleArithmetic(X, Y, N, NN, NNN);
      break;
    case 9:
      if (VX != VY) {
        PC_ += 2;
      }
      break;
    case 0xA:
      I_ = NNN;
      break;
    case 0xB:
      PC_ = (var_registers_[0] + NNN) % 255;
      break;
    case 0xC:
      // Random number between 0 and NN inclusive
      handleRandom(X, NN);
      break;
    case 0xD: {
      // Reset VF_
      VF_ = 0x00;

      bool did_pixel_turn_off = false;
      for (int i = 0; i < N; ++i) {
        did_pixel_turn_off =
            draw(VX % (width_), (VY + i) % height_, ram_[I_ + i]) ||
            did_pixel_turn_off;
      }

      if (did_pixel_turn_off) {
        VF_ = 0x01;
      }
    } break;
    case 0xE: {
      // Skip if key input
      bool is_key_pressed = get_key_state(VX);
      if ((NN == 0x9E && is_key_pressed) || (NN == 0xA1 && !is_key_pressed)) {
        PC_ += 1;
      }
    } break;
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
        case 0x0A: {
          // TODO: handle get input
          std::optional<int> key = get_key();
          if (!key.has_value()) {
            PC_ -= 2;
          }
        } break;
        case 0x29:
          // Handle font character.
          // Set I register to the address of character in VX.
          // We can just multiply by 5 since we are storing fonts at the start
          // of RAM and each font character is 5 bytes long.
          I_ = (VX)*5;
          break;
        case 0x33:
          handleBCD(VX);
          break;
        case 0x55:
          handleMemoryStore(X);
          break;
        case 0x65:
          handleMemoryLoad(X);
          break;
        default:
          break;
      }
    default:
      break;
  };
}

void Emulator::clear() {
  for (auto& pixel : pixels_) {
    pixel = 0;
  }
}

void Emulator::draw_util(const int x, const int y, sf::Color color) {
  const int x_actual = x * scale_;
  const int y_actual = y * scale_;

  sf::RectangleShape rectangle(sf::Vector2f(scale_, scale_));
  rectangle.setFillColor(color);
  rectangle.setPosition(x_actual, y_actual);

  window_->draw(rectangle);
}

void Emulator::draw_pixels() {
  int index = 0;
  for (const auto& pixel : pixels_) {
    sf::Color color = pixel == 0 ? sf::Color::White : sf::Color(150, 50, 250);

    int x = index % width_;
    int y = index / width_;

    draw_util(x, y, color);

    ++index;
  }
}

bool Emulator::draw(const int x, const int y, unsigned short int sprite) {
  int index = x + (y * width_);

  bool did_pixel_turn_off = false;
  int count = 0;
  while ((sprite & 0x00FF)) {
    const int pixel = sprite & 0x0080;
    sprite = sprite << 0x1;

    if (pixel) {
      if (!pixels_[index]) {
        pixels_[index] = 1;
      } else {
        pixels_[index] = 0;
        did_pixel_turn_off = true;
      }
    }

    count++;
    index++;
  }

  return did_pixel_turn_off;
}

bool Emulator::get_key_state(const int key) {
  return false;
}

std::optional<int> Emulator::get_key() {
  return std::nullopt;
}

// Function returns a random number between 0 and NN inclusive
void Emulator::handleRandom(const INSTR X, const INSTR NN) {
  std::random_device seeder;
  std::mt19937 rng(seeder());
  std::uniform_int_distribution<int> gen(
      0, NN);  // FIXME: Possible bug, check if this in inclusive

  const int random_num = gen(rng);
  var_registers_[X] = random_num;
}

void Emulator::handleBCD(const INSTR VX) {
  INSTR temp = VX;
  int offset = 0;
  while (temp) {
    INSTR digit = temp % 10;
    temp = temp / 10;

    ram_[I_ + (offset++)] = digit;
  }
}

void Emulator::handleMemoryLoad(const INSTR X) {
  for (INSTR i = 0; i <= X; ++i) {
    var_registers_[i] = ram_[I_ + i];
  }
}

void Emulator::handleMemoryStore(const INSTR X) {
  for (INSTR i = 0; i <= X; ++i) {
    ram_[I_ + i] = var_registers_[i];
  }
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
          : (X & 0x80) == 0x80 /* check if last bit is set, 10000000 -> 1*0
                                + 2*0 + 4*0 + 8*1 => 80 */
      ;

  VF_ = shifted_bit ? 1 : 0;
}