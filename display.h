#ifndef __DISPLAY_H_
#define __DISPLAY_H_

#include <optional>
#include <string>

#include <SDL2/SDL.h>
#include <SDL2/SDL_timer.h>

class Display {
 public:
  Display(const int width, const int height);
  ~Display() = default;

  // Delete the copy constructor and assignment operator
  Display(const Display&) = delete;
  Display& operator=(const Display&) = delete;

  // Output operations
  void clear();
  void draw(const int x, const int y, unsigned short int sprite);

  // Input operations
  bool get_key_state(const int key);
  std::optional<int> get_key();

 private:
  const int width_;
  const int height_;
};

#endif