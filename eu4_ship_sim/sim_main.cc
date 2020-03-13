#include <vector>
#include <iostream>

#include "absl/algorithm/container.h"
#include "absl/flags/flag.h"
#include "absl/random/random.h"

absl::BitGen bitgen;

enum ShipType {
  ST_HEAVY,
  ST_LIGHT,
  ST_GALLEY,
  ST_TRANSPORT,
};

struct TechBonus {
  int tech;
  int hull;
  int cannons;
};

struct Ship {
  Ship(ShipType st, int tech);
  double hull;
  int cannons;
  int width;

  int target;
  bool inland;

  void increase(int tech, const std::vector<TechBonus>& boni);
};

std::vector<TechBonus> heavies = {{3, 20, 40},  {9, 5, 10},   {15, 5, 10},
                                  {19, 10, 20}, {22, 10, 20}, {25, 10, 20}};
std::vector<TechBonus> lights = {{2, 8, 10}, {9, 2, 3},  {15, 2, 2},
                                 {19, 4, 5}, {23, 4, 5}, {26, 4, 5}};
std::vector<TechBonus> galleys = {{2, 8, 12}, {10, 2, 3}, {14, 2, 3},
                                  {18, 4, 6}, {21, 4, 6}, {24, 4, 6}};
std::vector<TechBonus> cogs = {{2, 12, 4}, {10, 3, 1}, {13, 3, 1},
                               {17, 6, 2}, {22, 6, 2}, {26, 6, 2}};

void Ship::increase(int tech, const std::vector<TechBonus>& boni) {
  for (const auto& b : boni) {
    if (tech < b.tech) {
      return;
    }
    hull += b.hull;
    cannons += b.cannons;
  }
}

Ship::Ship(ShipType st, int tech) {
  width = 1;
  inland = false;
  switch (st) {
    case ST_HEAVY:
      width = 3;
      increase(tech, heavies);
      break;
    case ST_LIGHT:
      increase(tech, lights);
      break;
    case ST_GALLEY:
      increase(tech, galleys);
      inland = true;
      break;
    case ST_TRANSPORT:
      increase(tech, cogs);
      break;
  }
}

struct Fleet {
  Fleet(int h, int l, int g, int t, int tech) {
    for (int i = 0; i < h; ++i) {
      ships.emplace_back(ST_HEAVY, tech);
    }
    for (int i = 0; i < l; ++i) {
      ships.emplace_back(ST_LIGHT, tech);
    }
    for (int i = 0; i < g; ++i) {
      ships.emplace_back(ST_GALLEY, tech);
    }
    for (int i = 0; i < t; ++i) {
      ships.emplace_back(ST_TRANSPORT, tech);
    }
  }
  std::vector<Ship> ships;
};

int setTargets(Fleet& fleet, int numTargets, double width) {
  int ret = 0;
  int weight = 0;
  for (auto& s : fleet.ships) {
    if (weight < width) {
      ++ret;
      s.target = absl::uniform_int_distribution<int>(1, numTargets)(bitgen) - 1;
      weight += s.width;
      continue;
    }
    s.target = -1;
  }
  return ret;
}

int removeSunk(std::vector<Ship>& ships) {
  int ret = 0;
  for (int i = 0; i < ships.size(); ++i) {
    if (ships[i].hull > 0) {
      continue;
    }
    ret++;
    ships[i] = ships.back();
    ships.pop_back();
  }
  return ret;
}

bool fight(Fleet one, Fleet two, bool inland) {
  const double width = 27.5;
  absl::c_shuffle(one.ships, bitgen);
  absl::c_shuffle(two.ships, bitgen);

  int numShips1 = setTargets(one, two.ships.size(), width);
  int numShips2 = setTargets(two, numShips1, width);
  setTargets(one, numShips2, width);

  int shipsSunk1 = 0;
  int shipsSunk2 = 0;

  // Assume loss if either the whole fleet is sunk, or ships equal to the whole
  // initial first rank are sunk.
  while (true) {
    for (auto& s : one.ships) {
      if (s.target < 0) {
        continue;
      }
      double damage = 0.05 * s.cannons;
      if (inland && s.inland) damage *= 2;
      two.ships[s.target].hull -= damage;
    }
    for (auto& s : two.ships) {
      if (s.target < 0) {
        continue;
      }
      double damage = 0.05 * s.cannons;
      if (inland && s.inland) damage *= 2;
      one.ships[s.target].hull -= damage;
    }
    int sunk1 = removeSunk(one.ships);
    shipsSunk1 += sunk1;
    int sunk2 = removeSunk(two.ships);
    shipsSunk2 += sunk2;

    if (shipsSunk1 >= numShips1 || one.ships.empty()) {
      return false;
    }
    if (shipsSunk2 >= numShips2 || two.ships.empty()) {
      return true;
    }

    if (sunk1 + sunk2 > 0) {
      int num = setTargets(one, two.ships.size(), width);
      num = setTargets(two, num, width);
      setTargets(one, num, width);
    }
  }
  
  return true;
}
  
int main(int argc, char** argv) {
  Fleet side1(1, 0, 3, 0, 8);
  Fleet side2(1, 0, 3, 0, 8);

  int wins = 0;
  for (int i = 0; i < 1000; ++i) {
    std::cout << "Round " << i << std::endl;
    if (fight(side1, side2, true)) {
      wins++;
    }
  }

  std::cout << "Wins for side 1: " << wins << std::endl;
  return 0;
}

