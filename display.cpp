#include <iostream>

#include "display.h"

Display::Display(const int width, const int height)
    : width_(width), height_(height) {
  if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
    std::cout << "Error initializing SDL: " << SDL_GetError() << std::endl;
  }

  pixels_.reserve(width * height);
}

void Display::blit() {
  int count = 0;
  for (const auto& pixel : pixels_) {
    std::cout << (pixel == 0 ? " " : "*");
    count++;

    if (count % width_) {
      std::cout << std::endl;
    }
  }
}

void Display::clear() {
  for (auto& pixel : pixels_) {
    pixel = 0;
  }
}

bool Display::draw(const int x, const int y, unsigned short int sprite) {
  int index = x + (y * width_);

  bool did_pixel_turn_off = false;
  while (sprite) {
    const int pixel = sprite & 0x80;
    sprite = sprite << 0x1;

    if (pixel) {
      if (!pixels_[index]) {
        pixels_[index] = 1;
      } else {
        pixels_[index] = 0;
        did_pixel_turn_off = true;
      }
    }

    index++;
  }

  return did_pixel_turn_off;
}

bool Display::get_key_state(const int key) {
  const Uint8* state = SDL_GetKeyboardState(NULL);
  return state[SDL_SCANCODE_RETURN] > 0;
}

std::optional<int> Display::get_key() {
  return std::nullopt;
}