#include "colony/interface/impl/text_interface.h"

#include <cstdlib>
#include <conio.h>
#include <functional>
#include <iostream>
#include <string>

#include "colony/controller/controller.h"

constexpr int rows = 60;
constexpr int columns = 150;

namespace interface {
namespace text {
namespace {

void clear(std::vector<std::string>* display) {
  for (auto& row : *display) {
    row = std::string(columns, ' ');
  }
}


void flip(const std::vector<std::string>& display) {
  if (system("CLS")) {
    system("clear");
  }
  for (const auto& c : display) {
    std::cout << c << "\n";
  }
}

}  // namespace

TextInterface::TextInterface(controller::GameControl* c)
    : display_(rows, std::string(columns, ' ')), quit_(false) {}

void TextInterface::awaitInput() {
  char input = ' ';
  while (!quit_) {
    input = _getch();
    handler_(input);
  }
}

void TextInterface::introHandler(char inp) {
  switch (inp) {
    case 'q':
    case 'Q':
      quit_ = true;
      break;
    case 'n':
    case 'N':
      newGameScreen();
      break;
    default:
      break;
  }
}

void TextInterface::newGameHandler(char inp) {
  switch (inp) {
    case 'b':
    case 'B':
      mainMenu();
      break;
    default:
      break;
  }
}

void TextInterface::output(int x, int y, const std::string& words) {
  if (y >= rows) return;
  for (int i = 0; i < words.size(); ++i) {
    if (i + x >= columns) {
      return;
    }
    display_[y][x+i] = words[i];
  }
}

void TextInterface::newGameScreen() {
  clear(&display_);
  handler_ = [&](char in) { this->newGameHandler(in); };
  flip(display_);
}

void TextInterface::mainMenu() {
  clear(&display_);
  handler_ = [&](char in) { this->introHandler(in); };
  output(20, 1, "    CCCCCCCCC      OOOOOOO     LLL           OOOOOOO     NNNNN     NNNN   YY       YY");
  output(20, 2, "  CCCCC    CCC    OOO   OOO    LLL          OOO   OOO    NNN NN     NNN    YY     YY");
  output(20, 3, " CCC             OOO     OOO   LLL         OOO     OOO   NNN  NN    NNN     YY   YY");
  output(20, 4, " CCC             OOO     OOO   LLL         OOO     OOO   NNN   NN   NNN      YY YY");
  output(20, 5, " CCC             OOO     OOO   LLL     L   OOO     OOO   NNN    NN  NNN       YYY");
  output(20, 6, "  CCCCC    CCC    OOO   OOO    LLLLLLLLL    OOO   OOO    NNN     NN NNN       YYY");
  output(20, 7, "   CCCCCCCCCC      OOOOOOO     LLLLLLLLL     OOOOOOO     NNNN     NNNNN       YYY");
  output(50, 20, "(N)ew game");
  output(50, 21, "(Q)uit");
  flip(display_);
}

void TextInterface::IntroScreen() {
  mainMenu();
  awaitInput();
}

}  // namespace text
}  // namespace interface
