chip8: emulator.h emulator.cpp main.cpp
	clang++ -std=c++17 -g -I/opt/homebrew/include/ -I./ -L/opt/homebrew/lib emulator.cpp -lsfml-graphics -lsfml-window -lsfml-system main.cpp -o chip8