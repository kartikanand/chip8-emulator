#include <iostream>

#include "display.h"

Display::Display(const int width, const int height)
    : width_(width), height_(height) {
  if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
    std::cout << "Error initializing SDL: " << SDL_GetError() << std::endl;
  }
}

void Display::clear() {}

void Display::draw(const int x, const int y, unsigned short int sprite) {}

bool Display::get_key_state(const int key) {
  const Uint8* state = SDL_GetKeyboardState(NULL);
  return state[SDL_SCANCODE_RETURN] > 0;
}

std::optional<int> Display::get_key() {
  return std::nullopt;
}