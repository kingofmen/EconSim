#include "games/sevenyears/constants.h"

namespace constants {

// TODO: Make 'inline' when we support C++17.
// TODO: Should these rather be defined in a proto somewhere?
const std::string& EuropeanTrade() {
  static const std::string kEuropeanTrade("european_trade");
  return kEuropeanTrade;
}
const std::string& ColonialTrade() {
  static const std::string kColonialTrade("colonial_trade");
  return kColonialTrade;
}
const std::string& SupplyArmies() {
  static const std::string kSupplyArmies("supply_armies");
  return kSupplyArmies;
}
const std::string& ImportCapacity() {
  static const std::string kImportCapacity("import_capacity");
  return kImportCapacity;
}
const std::string& LoadShip() {
  static const std::string kLoadShip("load_cargo");
  return kLoadShip;
}
const std::string& TradeGoods() {
  static const std::string kTradeGoods("trade_goods");
  return kTradeGoods;
}
const std::string& Supplies() {
  static const std::string kSupplies("supplies");
  return kSupplies;
}

}  // namespace constants

