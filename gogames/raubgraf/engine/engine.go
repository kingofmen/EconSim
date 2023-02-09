package engine

import (
  "fmt"

  "gogames/raubgraf/engine/board"
  "gogames/raubgraf/engine/pop"
  "gogames/raubgraf/engine/war"
)

const(
  maxProduction = 4
)

var (
  // TODO: Probably this should be in a config file.
  surplusProduction = []int{3, 2, 1}
  starveLevel = 3
  newPopFood = 2

  // banditryFail gives the fraction of bandits that cannot steal food
  // after each battle result.
  // Slightly off from precise fractions so that we don't rely on, e.g.,
  // 0.5*2 equalling 1 in floating-points.
  banditryFail = map[war.BattleResult]float64{
    war.Standoff: 1.01,
    war.BreakContact: 1.01,
    war.Disaster: 1.01,
    war.Loss: 0.799,
    war.MinorLoss: 0.667,
    war.Draw: 0.499,
    war.MinorVictory: -0.01,
    war.Victory: -0.01,
    war.Overrun: -0.01,
  }
)

type RaubgrafGame struct {
  world *board.Board
}

func (g *RaubgrafGame) valid() error {
  if g == nil {
    return fmt.Errorf("nil game object")
  }
  if g.world == nil {
    return fmt.Errorf("gameboard not created")
  }
  return nil
}

// processTriangles applies the provided function to every triangle.
func (g *RaubgrafGame) processTriangles(desc string, f func(t *board.Triangle) error) error {
  if err := g.valid(); err != nil {
    return fmt.Errorf("processTriangles(%s) validity error: %w", desc, err)
  }
  for i, t := range g.world.Triangles {
    if err := f(t); err != nil {
      return fmt.Errorf("while processing (%s) triangle %d: %w", desc, i, err)
    }
  }
  return nil
}

// processVertices applies the provided function to every vertex.
func (g *RaubgrafGame) processVertices(desc string, f func(v *board.Vertex) error) error {
  if err := g.valid(); err != nil {
    return fmt.Errorf("processTriangles(%s) validity error: %w", desc, err)
  }
  for x, verts := range g.world.Vertices {
    for y, v := range verts {
      if err := f(v); err != nil {
        return fmt.Errorf("while processing (%s) vertex (%d, %d): %w", desc, x, y, err)
      }
    }
  }
  return nil
}

// intMin returns the minimum of a and b.
func intMin(a, b int) int {
  if a < b {
    return a
  }
  return b
}

// produceFood creates food and places it into storage.
func produceFood(t *board.Triangle) error {
  if t == nil {
    return nil
  }
  pc := t.CountPops(pop.PeasantFilter, pop.AvailableFilter)
  t.ClearLand(pc)
  if !t.IsFarm() {
    return nil
  }

  // TODO: Effects of weather, improvements.
  production := intMin(pc, maxProduction)
  for i, extra := range surplusProduction {
    if i >= pc {
      break
    }
    production += extra
  }

  if err := t.AddFood(production); err != nil {
    return fmt.Errorf("produceFood error when storing food: %w", err)
  }

  return nil
}

// move processes Pop movement.
func move(t *board.Triangle) error {
  // TODO: Implement
  return nil
}

// mobilise returns the levies of the provided Pops.
func mobilise(pops []*pop.Pop) []*war.FieldUnit {
  ret := make([]*war.FieldUnit, 0, len(pops))
  for _, p := range pops {
    if u := p.Mobilise(); u != nil {
      ret = append(ret, u)
    }
  }
  return ret
}

// fight resolves any battles in the triangle.
func fight(t *board.Triangle) error {
  // Check for bandit raiding.
  bandits := t.FilteredPopulation(pop.BanditFilter, pop.FightingFilter)
  // Peasants are joined by any knights who have decided to defend this farm.
  peasants := t.FilteredPopulation(pop.KindsFilter(pop.Peasant, pop.Knight), pop.FightingFilter)

  if len(bandits) * len(peasants) > 0 {
    raiders, militia := mobilise(bandits), mobilise(peasants)
    // TODO: Casualties.
    // Battle result decides how many bandits get food.
    result := war.Battle(raiders, militia)
    hungryFrac := banditryFail[result]
    for idx, b := range bandits {
      if float64(idx) >= hungryFrac*float64(len(bandits)) {
        break
      }
      b.Work()
    }
  }

  return nil
}

// payRent makes peasants pay castles for protection.
func payRent(t *board.Triangle) error {
  if t == nil {
    return nil
  }

  con := t.GetContract()
  if con == nil {
    return nil
  }
  if err := con.Execute(); err != nil {
    // TODO: Probably the landlord should do something about this.
    return nil
  }
  return nil
}

// consumeLocalFood uses stored food to feed the local Pops.
func consumeLocalFood(t *board.Triangle) error {
  if t == nil {
    return nil
  }
  food := t.CountFood()
  pops := t.Population()
  available := make(map[pop.Kind]int)
  // Bandits marked 'busy' are the ones who were defeated in battle earlier.
  available[pop.Bandit] = intMin(food, t.CountPops(pop.BanditFilter, pop.AvailableFilter))
  food -= available[pop.Bandit]
  available[pop.Peasant] = intMin(food, t.CountKind(pop.Peasant))
  consumed := available[pop.Bandit] + available[pop.Peasant]

  for _, p := range pops {
    p.Eat(available[p.GetKind()] > 0)
    available[p.GetKind()]--
  }
  return t.AddFood(-consumed)
}

// banditry checks whether bandits steal outside their own triangle.
func banditry(t *board.Triangle) error {
  hungryBandits := t.FilteredPopulation(pop.BanditFilter, pop.HungryFilter)
  if len(hungryBandits) < 1 {
    return nil
  }

  eaten := 0
  for _, d := range board.TriDirs {
    neighbour := t.GetNeighbour(d)
    food := intMin(neighbour.CountFood(), len(hungryBandits)-eaten)
    if food < 1 {
      continue
    }
    if err := neighbour.AddFood(-food); err != nil {
      return fmt.Errorf("error when bandits in %v stole %d food from %v: %w", t, food, neighbour)
    }
    for _, b := range hungryBandits[eaten:eaten+food] {
      b.Eat(true)
    }
    eaten += food
    if len(hungryBandits) <= eaten {
      break
    }
  }

  return nil
}

// demographics removes starving pops and breeds well-fed ones.
func demographics(t *board.Triangle) error {
  if t == nil {
    return nil
  }

  hungry := t.FilteredPopulation(pop.HungryFilter)
  for _, pp := range hungry {
    if pp.GetHunger() < starveLevel {
      continue
    }
    t.RemovePop(pp)
  }

  if t.CountFood() < newPopFood {
    return nil
  }

  cand := t.FirstPop(pop.PeasantFilter, pop.NotHungryFilter)
  if cand == nil {
    return nil
  }
  if err := t.AddFood(-newPopFood); err != nil {
    return err
  }
  t.AddPop(cand.Copy())
  return nil
}

// ResolveTurn runs one tick of the game model.
func (g *RaubgrafGame) ResolveTurn() error {
  if err := g.valid(); err != nil {
    return fmt.Errorf("ResolveTurn validity error: %w", err)
  }

  if err := g.processTriangles("Produce food", produceFood); err != nil {
    return err
  }
  if err := g.processTriangles("Move", move); err != nil {
    return err
  }
  if err := g.processTriangles("Fight!", fight); err != nil {
    return err
  }
  if err := g.processTriangles("Pay rent", payRent); err != nil {
    return err
  }
  if err := g.processTriangles("Consume food", consumeLocalFood); err != nil {
    return err
  }
  if err := g.processTriangles("Banditry", banditry); err != nil {
    return err
  }
  if err := g.processTriangles("Demographics", demographics); err != nil {
    return err
  }
  return nil
}
