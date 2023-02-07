package war

import (
  "math"
  "sort"
)

type UnitState int
type BattleResult int

const(
  Fresh UnitState = iota
  Tired
  Battered
  Shattered
  Routed
  Massacred

  Standoff BattleResult = iota
  BreakContact
  Disaster
  Loss
  MinorLoss
  Draw
  MinorVictory
  Victory
  Overrun
)

// TODO: Put all the magic numbers in a config file.

// String returns a human-readable string for debugging;
// it is not intended for display to the user.
func (b BattleResult) String() string {
  switch b {
  case Standoff:
    return "Standoff"
  case BreakContact:
    return "BreakContact"
  case Disaster:
    return "Disaster"
  case Loss:
    return "Loss"
  case MinorLoss:
    return "MinorLoss"
  case Draw:
    return "Draw"
  case MinorVictory:
    return "MinorVictory"
  case Victory:
    return "Victory"
  case Overrun:
    return "Overrun"
  }
  return "Unknown"
}

// FieldUnit models a Pop mobilised for war.
type FieldUnit struct {
  drill int
  equip int
  mount int
  state UnitState
}

// modifier returns a combat-power modifier.
func (s UnitState) modifier() float64 {
  switch s {
  case Fresh:
    return 1.0
  case Tired:
    return 0.8
  case Battered:
    return 0.6
  case Shattered:
    return 0.33
  case Routed:
    return 0.1
  case Massacred:
    return 0.0
  }
  return 0.0
}

// unitPower returns the combat power of the unit.
// TODO: Config file.
func unitPower(u *FieldUnit) float64 {
  if u == nil {
    return 0.0
  }
  // Initially almost linear but with diminishing returns,
  // becoming noticeable around 5.
  ret := 0.1 + math.Erf(float64(u.equip)/6)
  // First increment of drill matters a lot, subsequent ones less.
  // 0.2, 0.6, 0.8, 0.9, 0.95
  ret *= (1.0 - 0.8*math.Pow(0.5, float64(u.drill)))
  if u.mount > 0 {
    ret *= 1.41
  }
  if u.mount > 5 {
    ret *= 1.41
  }

  return ret * u.state.modifier()
}

// scoutPower returns the scouting ability of the unit.
func scoutPower(u *FieldUnit) float64 {
  if u == nil {
    return 0.0
  }
  ret := 1.0
  if u.mount > 0 {
    ret *= 3
    // Penalise heavy cavalry
    if u.mount > 5 {
      ret *= 0.5
    }
  }
  // Penalise heavy armour.
  if u.equip > 4 {
    ret *= 0.75
  }
  if u.drill > 0 {
    ret *= 1.1
  }
  if u.drill > 3 {
    ret *= 1.1
  }

  return ret * u.state.modifier()
}

// combinedArms returns the combined-arms bonus, if any, of the
// provided unit distribution.
// TODO: Config file.
func combinedArms(inf, cav int) float64 {
  if inf*cav == 0 {
    return 1.0
  }

  // Symmetric payoffs, so just make infantry be the majority.
  if inf < cav {
    inf, cav = cav, inf
  }

  // In an army of 100 units, tiers kick in at: 10, 17, 20, 25, and 34.
  if 2*cav >= inf {
    return 1.50
  }
  if 3*cav >= inf {
    return 1.33
  }
  if 4*cav >= inf {
    return 1.25
  }
  if 5*cav >= inf {
    return 1.2
  }
  if 10*cav >= inf {
    return 1.15
  }

  return 1.0
}

// power returns the total combat and scouting power of the units.
func power(units []*FieldUnit) (float64, float64) {
  combat, scouting := 0.0, 0.0
  count := len(units)
  cavCount := 0
  scouts := make([]float64, 0, len(units))
  for _, u := range units {
    combat += unitPower(u)
    scouts = append(scouts, scoutPower(u))
    if u.mount > 0 {
      cavCount++
    }
  }
  infCount := count - cavCount
  combat *= combinedArms(infCount, cavCount)

  // Decreasing returns from scouting.
  sort.Float64s(scouts)
  for i := 0; i < count; i++ {
    scouting += scouts[count - 1 - i] * math.Pow(0.5, float64(i))
  }

  // TODO: There should be some sort of value to just Raw Numbers.
  return combat, scouting
}

// scoutModifier returns the advantage of the higher scouting power.
func scoutModifier(hi, lo float64) float64 {
  if hi == lo {
    return 1.0
  }
  // Symmetric, so calculate only one side.
  if hi < lo {
    hi, lo = lo, hi
  }

  if hi >= 10*lo {
    return 2.0
  }
  if hi >= 8*lo {
    return 1.66
  }
  if hi >= 6*lo {
    return 1.5
  }
  if hi >= 4*lo {
    return 1.33
  }
  if hi >= 3*lo {
    return 1.25
  }
  if hi >= 2*lo {
    return 1.16
  }
  return 1.1
}

// Battle returns true if the attackers win.
// TODO: Take terrain into account. Leadership even?
// TODO: Insert randomness here?
func Battle(attackers, defenders []*FieldUnit) BattleResult {
  if len(attackers) == 0 {
    return Standoff
  }
  if len(defenders) == 0 {
    return Standoff
  }

  attC, attS := power(attackers)
  defC, defS := power(defenders)
  scoutMod := scoutModifier(attS, defS)
  if attS > defS {
    attC *= scoutMod
  } else {
    defC *= scoutMod
  }

  // TODO: Calculate casualties.
  // TODO: Take aggression/desperation into account.
  if attC > 2*defC {
    // Defenders attempt retreat.
    if defS > 1.5*attS {
      return BreakContact
    }

    if attC > 3*defC {
      return Overrun
    }
    return Victory
  }

  if attC > 1.25*defC {
    return MinorVictory
  }

  if attC > 0.8*defC {
    // Could also be Standoff if not very aggressive.
    return Draw
  }

  if attC > 0.5*defC {
    return MinorLoss
  }
  
  // Attackers don't.
  if attS > 1.5 * defS {
    return Standoff
  }

  if attC > 0.333 * defC {
    return Loss
  }

  return Disaster
}
