#include "colony/interface/impl/text_interface.h"

#include <cstdlib>
#include <iostream>
#include <string>

#include "colony/controller/controller.h"

constexpr int rows = 60;
constexpr int columns = 150;


namespace interface {
namespace text {
namespace {

void flip(const std::vector<std::string>& display) {
  if (system("CLS")) {
    system("clear");
  }
  for (const auto& c : display) {
    std::cout << c << "\n";
  }
}

}  // namespace


TextInterface::TextInterface(controller::GameControl* c) : display_(rows, std::string(columns, ' ')) {}

void TextInterface::output(int x, int y, const std::string& words) {
  if (y >= rows) return;
  for (int i = 0; i < words.size(); ++i) {
    if (i + x >= columns) {
      return;
    }
    display_[y][x+i] = words[i];
  }
}


void TextInterface::IntroScreen() {
  output(20, 1, "    CCCCCCCCC      OOOOOOO     LLL           OOOOOOO     NNNNN     NNNN   YY       YY");
  output(20, 2, "  CCCCC    CCC    OOO   OOO    LLL          OOO   OOO    NNN NN     NNN    YY     YY");
  output(20, 3, " CCC             OOO     OOO   LLL         OOO     OOO   NNN  NN    NNN     YY   YY");
  output(20, 4, " CCC             OOO     OOO   LLL         OOO     OOO   NNN   NN   NNN      YY YY");
  output(20, 5, " CCC             OOO     OOO   LLL     L   OOO     OOO   NNN    NN  NNN       YYY");
  output(20, 6, "  CCCCC    CCC    OOO   OOO    LLLLLLLLL    OOO   OOO    NNN     NN NNN       YYY");
  output(20, 7, "   CCCCCCCCCC      OOOOOOO     LLLLLLLLL     OOOOOOO     NNNN     NNNNN       YYY");
  flip(display_);
}

}  // namespace text
}  // namespace interface
