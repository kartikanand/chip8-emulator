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

  void loop();

 private:
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

  std::unique_ptr<Display> display_;

  char ram_[4096];
  std::stack<INSTR> stack_;
  unsigned short int PC_;
  unsigned short int I_;
  int delay_timer_;
  int sound_timer_;
  int var_registers_[16];
  char VF_;
};

#endif
