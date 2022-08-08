#ifndef __EMULATOR_H_
#define __EMULATOR_H_

#include <stack>
#include <string>

#include "display.h"

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
  // Init function
  void init();

  // Control Unit
  INSTR fetch();
  void decode_and_execute(const INSTR current_instruction);

  // Audio
  void beep();

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

  std::unique_ptr<Display> display_;

  INSTR ram_[4096];
  std::stack<unsigned short int> stack_;
  unsigned short int PC_;
  unsigned short int I_;
  int delay_timer_;
  int sound_timer_;
  int var_registers_[16];
  unsigned char VF_;

  const unsigned short int program_start_address_ = 0x200;
};

#endif
