package engine

import (
  "fmt"
  "math"

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

type vFunc func(v *board.Vertex) error
type tFunc func(t *board.Triangle) error

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

// processPops applies the provided function to every Pop.
// This may occur in any order.
func (g *RaubgrafGame) processPops(desc string, f func(p *pop.Pop) error) error {
  if err := g.valid(); err != nil {
    return fmt.Errorf("processPops(%s) validity error: %w", desc, err)
  }

  count := 0
  for ti, t := range g.world.Triangles {
    tPops := t.Population()
    for pi, p := range tPops {
      count++
      if err := f(p); err != nil {
        return fmt.Errorf("while processing (%s) pop %d (%d in triangle %d): %w", desc, count, pi, ti, err)
      }
    }
  }
  for x, vtxs := range g.world.Vertices {
    for y, vtx := range vtxs {
      s := vtx.GetSettlement()
      if s == nil {
        continue
      }
      vPops := s.Population()
      for pi, p := range vPops {
        count++
        if err := f(p); err != nil {
          return fmt.Errorf("while processing (%s) pop %d (%d in vertex (%d, %d)): %w", desc, count, pi, x, y, err)
        }
      }
    }
  }
  return nil
}

// processBoard applies the provided functions to every vertex and triangle.
// If tFirst is set, triangles are processed before vertices.
func (g *RaubgrafGame) processBoard(desc string, vf vFunc, tf tFunc, tFirst bool) error {
  if tFirst {
    if err := g.processTriangles(desc, tf); err != nil {
      return err
    }
  }
  if err := g.processVertices(desc, vf); err != nil {
    return err
  }
  if !tFirst {
    if err := g.processTriangles(desc, tf); err != nil {
      return err
    }
  }
  return nil
}

// processTriangles applies the provided function to every triangle.
func (g *RaubgrafGame) processTriangles(desc string, f tFunc) error {
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
func (g *RaubgrafGame) processVertices(desc string, f vFunc) error {
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

// popsThinkT runs the Pop AI (for Triangle denizens) where not
// overridden by incoming commands.
func popsThinkT(t *board.Triangle) error {
  // Peasants decide whether to work or defend against bandit raids.
  peasants := t.FilteredPopulation(pop.PeasantFilter)
  bandits := t.FilteredPopulation(pop.BanditFilter)

  // Peasants want to maximise effective food production.
  // They will assume that a militia unit is roughly equal
  // to a bandit unit.
  // TODO: More precise levy-power estimates, but also, less precise
  // knowledge of bandit strength!
  nBandits := len(bandits)
  banditStr := float64(nBandits)
  nWorkers := len(peasants)
  maxProd := 0
  for work := range peasants {
    currProd := foodAmount(work+1)
    peasantStr := float64(len(peasants) - (work+1))
    res := war.Result(banditStr, banditStr, peasantStr, peasantStr)
    stolen := nBandits - beatenBandits(nBandits, res)
    currProd -= stolen
    if currProd < maxProd {
      continue
    }
    maxProd = currProd
    nWorkers = work+1
  }
  for _, p := range peasants[nWorkers:] {
    p.SetAction(pop.Levy)
  }

  // TODO: Appeal to nearby buildings for help.
  // TODO: Go elsewhere if losing badly - also applies to bandits.

  return nil
}

// popsThinkV runs the Pop AI (for Vertex inhabitants) where not
// overridden by incoming commands.
func popsThinkV(v *board.Vertex) error {
  // TODO: Implement.
  return nil
}

// foodAmount returns the amount of food created by the given number of peasants.
func foodAmount(pc int) int {
  production := intMin(pc, maxProduction)
  for i, extra := range surplusProduction {
    if i >= pc {
      break
    }
    production += extra
  }
  return production
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
  production := foodAmount(pc)
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

// beatenBandits returns the number of bandits who cannot steal
// food because they were driven off by defenders.
func beatenBandits(nb int, res war.BattleResult) int {
  return int(math.Round(banditryFail[res] * float64(nb)))
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
    beaten := beatenBandits(len(bandits), result)
    for _, b := range bandits[:beaten] {
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
    if neighbour == nil {
      continue
    }
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

  if err := g.processPops("Clear pops", func(p *pop.Pop) error {
    p.Clear()
    return nil
  }); err != nil {
    return err
  }
  if err := g.processBoard("Pop thinking", popsThinkV, popsThinkT, true); err != nil {
    return err
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
