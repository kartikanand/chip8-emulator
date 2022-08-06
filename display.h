#ifndef __DISPLAY_H_
#define __DISPLAY_H_

#include <string>

class Display {
 public:
  Display(const int width, const int height);
  ~Display() = default;

  // Delete the copy constructor and assignment operator
  Display(const Display&) = delete;
  Display& operator=(const Display&) = delete;

  void clear();

  void draw(const int x, const int y, unsigned short int sprite);

 private:
  const int width_;
  const int height_;
};

#endif