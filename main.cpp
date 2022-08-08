#include <iostream>
#include "emulator.h"

int main(int argc, char** argv) {
  if (argc < 2) {
    std::cout << "Please specify a program file" << std::endl;
    return 1;
  }

  const std::string file_name = std::string(argv[1]);
  const std::unique_ptr<Emulator> emulator = std::make_unique<Emulator>();

  emulator->load_program(file_name);

  emulator->loop();

  return 0;
}