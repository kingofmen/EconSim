#include <cmath>
#include <cstdio>
#include <iomanip>
#include <iostream>
#include <string>
#include <unordered_map>

#include "google/protobuf/text_format.h"
#include "google/protobuf/io/zero_copy_stream_impl.h"
#include "mp_rankings/proto/rankings.pb.h"
#include "util/proto/file.h"

struct rating {
  double score;
  double deviation;
  int recentConflict;
  double mostRecentDelta;
  int previousRank;
};

std::unordered_map<std::string, rating> scores;
std::unordered_map<std::string, rankings::proto::PlayerInfo> player_info;

constexpr double kNewPlayerScore = 1500.0;
constexpr double kNewPlayerDeviation = 350.0;
constexpr double kDecayConstantSq = 33.3 * 33.3;
constexpr double kQ = 0.00575646273; // ln(10) / 400, from Wiki on Glicko system.
constexpr double kQ2 = kQ * kQ;
constexpr double kPi = 3.14159265;

constexpr int FG_BOLD = 1;
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
    {FG_BOLD, "1"},    {FG_BLACK, "30"}, {FG_RED, "31"},     {FG_GREEN, "32"},
    {FG_YELLOW, "33"}, {FG_BLUE, "34"},  {FG_MAGENTA, "35"}, {FG_CYAN, "36"},
    {FG_WHITE, "37"},  {BG_BLACK, "40"}, {BG_RED, "41"},     {BG_GREEN, "42"},
    {BG_YELLOW, "43"}, {BG_BLUE, "44"},  {BG_MAGENTA, "45"}, {BG_CYAN, "46"},
    {BG_WHITE, "47"},
};


// Returns the weighted rating and weight for a player.
std::pair<double, double>
adjustedScore(const rankings::proto::Player& player) {
  auto& rate = scores[player.name()];
  double weight = player.commitment() / rate.deviation;
  return {rate.score * weight, weight};
}

// Returns the team rating and deviation for a set of players, equal to the
// commitment-weighted average for rating and the plain average for deviation.
rating
initialise(const google::protobuf::RepeatedPtrField<rankings::proto::Player>& players,
           int session) {
  double avgRating = 0;
  double avgDev = 0;
  double totalCommit = 0;
  for (const auto& player : players) {
    std::string name = player.name();
    if (scores.find(name) == scores.end()) {
      scores[name] = {kNewPlayerScore, kNewPlayerDeviation, session, 0};
    }

    auto& rate = scores[name];
    avgRating += rate.score * player.commitment();
    totalCommit += player.commitment();

    rate.deviation =
        std::min(sqrt(pow(rate.deviation, 2) +
                      kDecayConstantSq * (session - rate.recentConflict)),
                 kNewPlayerDeviation);
    avgDev += rate.deviation;
  }
  if (totalCommit < 0.00001) {
    return {-1, -1, 0};
  }

  avgRating /= totalCommit;
  avgDev /= players.size();
  return {avgRating, avgDev, 0, 0};
}

double g(double RDi) { return 1.0 / sqrt(1 + 3 * pow(kQ * RDi / kPi, 2)); }

double E(double r0, double ri, double RDi) {
  double grdi = g(RDi);
  return 1.0 / (1.0 + pow(10, -0.0025 * grdi * (r0 - ri)));
}

double d2(double r0, double ri, double RDi) {
  double grdi = g(RDi);
  double expect = E(r0, ri, RDi);
  return 1.0 / (kQ2 * grdi * grdi * expect * (1.0 - expect));
}

double deltaGlicko(const rating& player, const rating& opponent, double score) {
  double grdi = g(opponent.deviation);
  double expect = E(player.score, opponent.score, opponent.deviation);
  double dSquared = d2(player.score, opponent.score, opponent.deviation);
  double inverse = 1.0 / dSquared + 1.0 / pow(player.deviation, 2);
  return (kQ / inverse) * grdi * (score - expect);
}

std::string escapeCode(int mask) {
  if (mask == 0) {
    return "\033[0m";
  }
  std::vector<std::string> codes;
  for (const auto& code : ansiCodes) {
    if (code.first & mask) {
      codes.push_back(code.second);
    }
  }
  std::string ret = "\033[";
  for (int i = 0; i < codes.size(); ++i) {
    ret += codes[i];
    ret += (i == codes.size() - 1 ? 'm' : ';');
  }
  return ret;
}

std::string formatName(const std::string name, int rank = 0, int priorRank = 0) {
  static char buffer[10000];
  if (rank > 0) {
    if (priorRank > 0) {
      int delta = priorRank - rank;
      if (delta == 0) {
        sprintf(buffer, "%2d      %-15s", rank, name.c_str());
      } else {
        sprintf(buffer, "%2d (%+d) %-15s", rank, delta, name.c_str());
      }
    } else {
      sprintf(buffer, "%2d. %-15s", rank, name.c_str());
    }
  } else {
    sprintf(buffer, "%-15s", name.c_str());
  }
  return std::string(buffer);
}

void calculateRank(const rankings::proto::Conflict& conflict) {
  std::string name = "Unnamed conflict";
  if (conflict.has_name()) {
    name = conflict.name();
  }
  std::cout << name << "\n";

  auto avgAttRate = initialise(conflict.attackers(), conflict.session());
  if (avgAttRate.score < 0) {
    std::cout << "  No committed attacker, skipping\n";
    return;
  }
  auto avgDefRate = initialise(conflict.defenders(), conflict.session());
  if (avgDefRate.score < 0) {
    std::cout << "  No committed defender, skipping\n";
    return;
  }

  double attackerDelta = deltaGlicko(avgAttRate, avgDefRate, conflict.attacker_win());
  double defenderDelta = deltaGlicko(avgDefRate, avgAttRate, 1.0 - conflict.attacker_win());
  
  double attackingPlayers = 0;
  double defendingPlayers = 0;
  for (const auto& player : conflict.attackers()) {
    attackingPlayers += player.commitment();
  }
  for (const auto& player : conflict.defenders()) {
    defendingPlayers += player.commitment();
  }
  defendingPlayers *= (1.0 - conflict.vulture_factor());

  for (const auto& player : conflict.attackers()) {
    auto& rank = scores[player.name()];
    double playerFraction = rank.score * player.commitment();
    playerFraction /= avgAttRate.score;
    double playerDelta = playerFraction * attackerDelta;
    double ratio = defendingPlayers / attackingPlayers;
    if (attackerDelta < 0) {
      ratio = attackingPlayers / defendingPlayers;
    }
    playerDelta *= ratio;

    std::cout << "  " << formatName(player.name()) << ": " << rank.score << " -> "
              << rank.score + playerDelta << "\n";
    double dSquare = d2(rank.score, avgDefRate.score, avgDefRate.deviation);
    rank.score += playerDelta;
    if (conflict.session() > rank.recentConflict) {
      rank.mostRecentDelta = playerDelta;
    } else if (conflict.session() == rank.recentConflict) {
      rank.mostRecentDelta += playerDelta;
    }
    rank.recentConflict = conflict.session();
    rank.deviation = sqrt(pow(pow(rank.deviation, -2) + pow(dSquare, -1), -1));
  }

  for (const auto& player : conflict.defenders()) {
    auto& rank = scores[player.name()];
    double playerFraction = rank.score * player.commitment();
    playerFraction /= avgDefRate.score;
    double playerDelta = playerFraction * defenderDelta;
    double ratio = attackingPlayers / defendingPlayers;
    if (defenderDelta < 0) {
      ratio = defendingPlayers / attackingPlayers;
    }
    playerDelta *= ratio;
    std::cout << "  " << formatName(player.name()) << ": " << rank.score << " -> "
              << rank.score + playerDelta << "\n";
    double dSquare = d2(rank.score, avgAttRate.score, avgAttRate.deviation);
    rank.score += playerDelta;
    if (conflict.session() > rank.recentConflict) {
      rank.mostRecentDelta = playerDelta;
    } else if (conflict.session() == rank.recentConflict) {
      rank.mostRecentDelta += playerDelta;
    }
    rank.recentConflict = conflict.session();    
    rank.deviation = sqrt(pow(pow(rank.deviation, -2) + pow(dSquare, -1), -1));
  }
}

int main(int argc, char** argv) {
  if (argc == 1) {
    std::cout << "Must specify input file\n";
    return 1;
  }

  rankings::proto::Ranking ranking;
  auto status = util::proto::ParseProtoFile(argv[1], &ranking);
  if (!status.ok()) {
    std::cout << status.error_message() << "\n";
    return 1;
  }
  for (const auto& info : ranking.player_infos()) {
    player_info[info.name()] = info;
  }

  auto* sorted = ranking.mutable_conflicts();
  int latestSession = 0;
  std::sort(sorted->begin(), sorted->end(),
            [](const rankings::proto::Conflict& one,
               const rankings::proto::Conflict& two) {
              return one.session() < two.session();
            });
  for (const auto& conflict : ranking.conflicts()) {
    if (conflict.session() > latestSession) {
      latestSession = conflict.session();
    }
    calculateRank(conflict);
  }

  std::vector<std::string> names;
  for (auto& it : scores) {
    if (it.second.recentConflict != latestSession) {
      it.second.mostRecentDelta = 0;
    }
    names.push_back(it.first);
  }

  std::sort(names.begin(), names.end(),
            [](const std::string& one, const std::string& two) {
              return scores[one].score - scores[one].mostRecentDelta >
                     scores[two].score - scores[two].mostRecentDelta;
            });
  int rank = 0;
  for (const auto& name : names) {
    if (!player_info[name].active()) {
      continue;
    }
    scores[name].previousRank = ++rank;
  }

  std::sort(names.begin(), names.end(),
            [](const std::string& one, const std::string& two) {
              return scores[one].score > scores[two].score;
            });

  std::cout << std::fixed << std::setprecision(2);
  rank = 0;
  for (const auto& name : names) {
    if (!player_info[name].active()) {
      continue;
    }
    ++rank;
    auto& currScore = scores[name];
    if (currScore.previousRank != rank) {
      std::cout << escapeCode(currScore.previousRank > rank ? FG_GREEN : FG_RED);
    }
    std::cout << formatName(name, rank, currScore.previousRank) << ": "
              << currScore.score << " (" << currScore.deviation << ")";
    if (currScore.recentConflict == latestSession) {
      std::cout << (currScore.mostRecentDelta > 0 ? " +" : " ")
                << currScore.mostRecentDelta;
    }
    std::cout << escapeCode(0);
    std::cout << "\n";
  }

  return 0;
}
