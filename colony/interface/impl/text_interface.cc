#include "colony/interface/impl/text_interface.h"

#include <algorithm>
#include <cstdlib>
#include <conio.h>
#include <experimental/filesystem>
#include <functional>
#include <iostream>
#include <string>

#include "absl/strings/substitute.h"
#include "absl/strings/str_join.h"
#include "colony/controller/controller.h"
#include "game/game_world.h"
#include "game/proto/game_world.pb.h"
#include "game/setup/proto/setup.pb.h"
#include "game/validation/validation.h"
#include "util/proto/file.h"
#include "util/status/status.h"

constexpr int rows = 60;
constexpr int columns = 150;
constexpr int messageLine = 59;

namespace interface {
namespace text {
namespace {

constexpr int BOLD = 1;
constexpr int FG_BLACK = 2;
constexpr int FG_RED = 4;
constexpr int FG_GREEN = 8;
constexpr int FG_YELLOW = 16;
constexpr int FG_BLUE = 32;
constexpr int FG_MAGENTA = 64;
constexpr int FG_CYAN = 128;
constexpr int FG_WHITE = 256;
constexpr int BG_BLACK = 512;
constexpr int BG_RED = 1024;
constexpr int BG_GREEN = 2048;
constexpr int BG_YELLOW = 4096;
constexpr int BG_BLUE = 8192;
constexpr int BG_MAGENTA = 16384;
constexpr int BG_CYAN = 32768;
constexpr int BG_WHITE = 65536;

std::unordered_map<int, std::string> ansiCodes = {
    {BOLD, "1"},       {FG_BLACK, "30"}, {FG_RED, "31"},     {FG_GREEN, "32"},
    {FG_YELLOW, "33"}, {FG_BLUE, "34"},  {FG_MAGENTA, "35"}, {FG_CYAN, "36"},
    {FG_WHITE, "37"},  {BG_BLACK, "40"}, {BG_RED, "41"},     {BG_GREEN, "42"},
    {BG_YELLOW, "43"}, {BG_BLUE, "44"},  {BG_MAGENTA, "45"}, {BG_CYAN, "46"},
    {BG_WHITE, "47"},
};

template <typename T>
bool select(char inp, const std::vector<T>& options, T& output) {
  if (options.size() == 0) {
    return false;
  }
  int idx = atoi(std::string(1, inp).c_str()) - 1;
  if (idx < 0 || idx >= options.size()) {
    return false;
  }
  output = options[idx];
  return true;
}

std::vector<std::experimental::filesystem::path> getScenarios() {
  auto current_path = std::experimental::filesystem::current_path();
  current_path /= "scenarios";

  if (!std::experimental::filesystem::exists(current_path)) {
    return {};
  }
  auto file_it = std::experimental::filesystem::directory_iterator(current_path);
  auto end = std::experimental::filesystem::directory_iterator();
  std::vector<std::experimental::filesystem::path> scenarios;
  for (; file_it != end; ++file_it) {
    if (file_it->path().extension() != ".scenario") {
      continue;
    }
    scenarios.push_back(file_it->path());
  }

  return scenarios;
}

}  // namespace

TextInterface::TextInterface(controller::GameControl* c)
    : display_(rows, std::string(columns, ' ')), colours_(rows, {}),
      quit_(false) {}

template <typename T>
void TextInterface::addSelection(int x, int y, const std::vector<T>& options,
                                 std::function<std::string(const T&)> toString) {
  int idx = 0;
  for (const T& opt : options) {
    output(x, y + idx, 0, absl::Substitute("($0) $1", idx + 1, toString(opt)));
    idx++;
  }
}

void TextInterface::awaitInput() {
  char input = ' ';
  while (!quit_) {
    input = _getch();
    handler_(input);
  }
}

void TextInterface::clearLine(int line) {
  display_[line] = std::string(columns, ' ');
  colours_[line].clear();
}

void TextInterface::clear() {
  for (int i = 0; i < rows; ++i) {
    clearLine(i);
  }
}

void TextInterface::drawWorld() {
  clear();
  flip();
}

void TextInterface::errorMessage(const std::string& error) {
  clearLine(messageLine);
  output(1, messageLine, FG_RED, error);
  flip();
}

std::string escapeCode(int mask) {
  if (mask == 0) {
    return "\033[0m";
  }
  std::vector<std::string> codes;
  for (const auto& code : ansiCodes) {
    if (mask & code.first) {
      codes.push_back(code.second);
    }
  }
  return absl::Substitute("\033[$0m", absl::StrJoin(codes, ";"));
}

void TextInterface::flip() const {
  if (system("CLS")) {
    system("clear");
  }
  for (int line = 0; line < rows; ++line) {
    int start = 0;
    std::cout << "\033[0m";
    for (const auto& col : colours_[line]) {
      int end = std::get<0>(col);
      std::cout << display_[line].substr(start, end - start);
      std::cout << escapeCode(std::get<1>(col));
      start = end;
    }
    std::cout << display_[line].substr(start) << "\n";
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
    case 'l':
    case 'L':
      loadGameScreen();
      break;
    default:
      break;
  }
}

void TextInterface::loadGameHandler(char inp) {
  switch (inp) {
    case 'b':
    case 'B':
      mainMenu();
      break;
    case 'q':
    case 'Q':
      quit_ = true;
      break;
    default:
      break;
  }
}

google::protobuf::util::Status
TextInterface::loadScenario(const game::setup::proto::ScenarioFiles& setup) {
  if (!setup.has_world_file()) {
    return util::InvalidArgumentError(
        absl::Substitute("$0 has no world_file", setup.name()));
  }

  scenario_.Clear();
  std::experimental::filesystem::path base_path = setup.root_path();
  for (const auto& filename : setup.auto_production()) {
    std::experimental::filesystem::path full_path = base_path / filename;
    auto status = util::proto::MergeProtoFile(full_path.string(), &scenario_);
    if (!status.ok()) {
      return status;
    }
  }
  for (const auto& filename : setup.production_chains()) {
    std::experimental::filesystem::path full_path = base_path / filename;
    auto status = util::proto::MergeProtoFile(full_path.string(), &scenario_);
    if (!status.ok()) {
      return status;
    }
  }
  for (const auto& filename : setup.trade_goods()) {
    std::experimental::filesystem::path full_path = base_path / filename;
    auto status = util::proto::MergeProtoFile(full_path.string(), &scenario_);
    if (!status.ok()) {
      return status;
    }
  }
  for (const auto& filename : setup.consumption()) {
    std::experimental::filesystem::path full_path = base_path / filename;
    auto status = util::proto::MergeProtoFile(full_path.string(), &scenario_);
    if (!status.ok()) {
      return status;
    }
  }
  for (const auto& filename : setup.unit_templates()) {
    std::experimental::filesystem::path full_path = base_path / filename;
    auto status = util::proto::MergeProtoFile(full_path.string(), &scenario_);
    if (!status.ok()) {
      return status;
    }
  }

  game_world_.Clear();
  std::experimental::filesystem::path world_path = base_path / setup.world_file();
  auto status = util::proto::ParseProtoFile(world_path.string(), &game_world_);
  if (!status.ok()) {
    return status;
  }

  std::vector<std::string> errors = game::validation::Validate(scenario_, game_world_);
  if (!errors.empty()) {
    return util::InvalidArgumentError(errors[0]);
  }

  return util::OkStatus();
}

void TextInterface::newGameHandler(char inp) {
  game::setup::proto::ScenarioFiles setup;
  if (select(inp, scenario_files_, setup)) {
    auto status = loadScenario(setup);
    if (status.ok()) {
      world_model_ = std::make_unique<game::GameWorld>(game_world_, &scenario_);
      gameDisplay();
    } else {
      errorMessage(status.error_message());
    }
    return;
  }

  switch (inp) {
    case 'b':
    case 'B':
      mainMenu();
      break;
    case 'q':
    case 'Q':
      quit_ = true;
      break;
    default:
      break;
  }
}

void TextInterface::runGameHandler(char inp) {
  switch (inp) {
    case 'b':
    case 'B':
      mainMenu();
      break;
    case 'q':
    case 'Q':
      quit_ = true;
      break;
    default:
      break;
  }  
}

void TextInterface::output(int x, int y, int mask, const std::string& words) {
  if (y >= rows) return;
  if (x >= columns) return;
  for (int i = 0; i < words.size(); ++i) {
    if (i + x >= columns) {
      return;
    }
    display_[y][x+i] = words[i];
  }
  if (mask != 0) {
    colours_[y].emplace_back(x, mask);
    if (x + words.size() < columns) {
      colours_[y].emplace_back(x + (int) words.size(), 0);
    }
    std::sort(
        colours_[y].begin(), colours_[y].end(),
        [](const std::tuple<int, int>& one, const std::tuple<int, int>& two) {
          return std::get<0>(one) < std::get<0>(two);
        });
  }
}

void TextInterface::gameDisplay() {
  handler_ = [&](char in) { this->runGameHandler(in); };
  drawWorld();
}

void TextInterface::loadGameScreen() {
  clear();
  handler_ = [&](char in) { this->loadGameHandler(in); };
  flip();
}

void TextInterface::newGameScreen() {
  clear();
  handler_ = [&](char in) { this->newGameHandler(in); };
  output(30, 2, 0, "Select scenario to play:");
  output(30, 10, 0, "(B)ack");
  output(30, 11, 0, "(Q)uit");
  const auto filepaths = getScenarios();
  scenario_files_.clear();

  for (const auto& path : filepaths) {
    scenario_files_.emplace_back();
    auto status = util::proto::ParseProtoFile(path.string(), &scenario_files_.back());
    if (!status.ok()) {
      scenario_files_.back().set_name(absl::Substitute("Error reading file $0", path.filename().string()));
    }
    if (scenario_files_.back().name().empty()) {
      scenario_files_.back().set_name(path.filename().string());
    }
    if (!scenario_files_.back().has_root_path()) {
      scenario_files_.back().set_root_path(path.parent_path().string());
    }
  }
  if (scenario_files_.empty()) {
    output(30, 12, 0, "No scenarios found");
  } else {
    addSelection<game::setup::proto::ScenarioFiles>(
        30, 13, scenario_files_,
        [](const game::setup::proto::ScenarioFiles& input) {
          return input.name();
        });
  }
  flip();
}

void TextInterface::mainMenu() {
  clear();
  handler_ = [&](char in) { this->introHandler(in); };
  output(20, 1, 0, "    CCCCCCCCC      OOOOOOO     LLL           OOOOOOO     NNNNN     NNNN   YY       YY");
  output(20, 2, 0, "  CCCCC    CCC    OOO   OOO    LLL          OOO   OOO    NNN NN     NNN    YY     YY");
  output(20, 3, 0, " CCC             OOO     OOO   LLL         OOO     OOO   NNN  NN    NNN     YY   YY");
  output(20, 4, 0, " CCC             OOO     OOO   LLL         OOO     OOO   NNN   NN   NNN      YY YY");
  output(20, 5, 0, " CCC             OOO     OOO   LLL     L   OOO     OOO   NNN    NN  NNN       YYY");
  output(20, 6, 0, "  CCCCC    CCC    OOO   OOO    LLLLLLLLL    OOO   OOO    NNN     NN NNN       YYY");
  output(20, 7, 0, "   CCCCCCCCCC      OOOOOOO     LLLLLLLLL     OOOOOOO     NNNN     NNNNN       YYY");
  output(50, 20, 0, "(N)ew game");
  output(50, 21, 0, "(L)oad game (not implemented)");
  output(50, 22, 0, "(Q)uit");
  flip();
}

void TextInterface::IntroScreen() {
  mainMenu();
  awaitInput();
}

}  // namespace text
}  // namespace interface
