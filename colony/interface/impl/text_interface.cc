#include "colony/interface/impl/text_interface.h"

#include <cstdlib>
#include <conio.h>
#include <experimental/filesystem>
#include <functional>
#include <iostream>
#include <string>

#include "absl/strings/substitute.h"
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

void clearLine(int line, std::vector<std::string>* display) {
  (*display)[line] = std::string(columns, ' ');
}

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
    : display_(rows, std::string(columns, ' ')), quit_(false) {}

template <typename T>
void TextInterface::addSelection(int x, int y, const std::vector<T>& options,
                                 std::function<std::string(const T&)> toString) {
  int idx = 0;
  for (const T& opt : options) {
    output(x, y + idx, absl::Substitute("($0) $1", idx + 1, toString(opt)));
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

void TextInterface::drawWorld() {
  clear(&display_);
  flip(display_);
}

void TextInterface::errorMessage(const std::string& error) {
  clearLine(messageLine, &display_);
  output(1, messageLine, absl::Substitute("\033[31m$0\033[0m", error));
  flip(display_);
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

void TextInterface::output(int x, int y, const std::string& words) {
  if (y >= rows) return;
  for (int i = 0; i < words.size(); ++i) {
    if (i + x >= columns) {
      return;
    }
    display_[y][x+i] = words[i];
  }
}

void TextInterface::gameDisplay() {
  handler_ = [&](char in) { this->runGameHandler(in); };
  drawWorld();
}

void TextInterface::loadGameScreen() {
  clear(&display_);
  handler_ = [&](char in) { this->loadGameHandler(in); };
  flip(display_);
}

void TextInterface::newGameScreen() {
  clear(&display_);
  handler_ = [&](char in) { this->newGameHandler(in); };
  output(30, 2, "Select scenario to play:");
  output(30, 10, "(B)ack");
  output(30, 11, "(Q)uit");
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
    output(30, 12, "No scenarios found");
  } else {
    addSelection<game::setup::proto::ScenarioFiles>(
        30, 13, scenario_files_,
        [](const game::setup::proto::ScenarioFiles& input) {
          return input.name();
        });
  }
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
  output(50, 21, "(L)oad game (not implemented)");
  output(50, 22, "(Q)uit");
  flip(display_);
}

void TextInterface::IntroScreen() {
  mainMenu();
  awaitInput();
}

}  // namespace text
}  // namespace interface
