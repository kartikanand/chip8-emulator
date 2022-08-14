#ifndef __EMULATOR_H_
#define __EMULATOR_H_

#include <stack>
#include <string>

#include <SFML/Graphics.hpp>

using INSTR = unsigned short int;

class Emulator {
 public:
  Emulator();
  ~Emulator() = default;

  // Delete the copy constructor and assignment operator
  Emulator(const Emulator&) = delete;
  Emulator& operator=(const Emulator&) = delete;

  // Takes a program and loads it in memory
  void load_program(const std::string& file_name);

  // Loop!
  void loop();

 private:
  // Reset function
  void reset();

  // Control Unit
  INSTR fetch();
  void decode_and_execute(const INSTR current_instruction);

  // Audio, Video, Input
  void beep();
  void clear();
  void draw_util(const int x, const int y, sf::Color color);
  void draw_pixels();
  bool draw(const int x, const int y, unsigned short int sprite);
  bool get_key_state(const int key);
  std::optional<int> get_key();

  // Arithmetic functions
  void handleArithmetic(const INSTR X,
                        const INSTR Y,
                        const INSTR N,
                        const INSTR NN,
                        const INSTR NNN);
  void handleAdd(const INSTR X, const INSTR NN);
  void handleSubtract(const INSTR X, const INSTR Y, const INSTR N);
  void handleShift(const INSTR X, const INSTR NN);

  // Speciality functions
  void handleRandom(const INSTR X, const INSTR NN);
  void handleBCD(const INSTR NN);
  void handleMemoryLoad(const INSTR X);
  void handleMemoryStore(const INSTR X);

  std::unique_ptr<sf::RenderWindow> window_;

  INSTR ram_[4096];
  std::stack<unsigned short int> stack_;
  unsigned short int PC_;
  unsigned short int I_;
  int delay_timer_;
  int sound_timer_;
  int var_registers_[16];
  unsigned char VF_;

  const unsigned short int program_start_address_ = 0x200;
  const int width_ = 64;
  const int height_ = 32;
  const int scale_ = 20;
  std::vector<int> pixels_;
};

#endif
