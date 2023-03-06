package engine

import (
  "fmt"
  "math"
  "math/rand"

  "gogames/raubgraf/engine/board"
  "gogames/raubgraf/engine/graph"
  "gogames/raubgraf/engine/pop"
  "gogames/raubgraf/engine/war"
  "gogames/util/flags"
)

const(
  maxProduction = 4
)

var (
  // TODO: Probably this should be in a config file.
  surplusFood = []int{3, 2, 1}
  wealthProduction = []int{5, 10}
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

  // flagDecay gives the amount each flag changes by per turn.
  flagDecay = map[string]int{
    board.BeaconFlag: -1,
  }
)

type vFunc func(v *board.Vertex) error
type tFunc func(t *board.Triangle) error

type RaubgrafGame struct {
  dice *rand.Rand
  world *board.Board
  decay *flags.Holder
  units []*war.FieldUnit
}

// NewGame returns a new game object.
func NewGame(w *board.Board) *RaubgrafGame {
  g := &RaubgrafGame{
    dice: rand.New(rand.NewSource(3)),
    world: w,
    decay: flags.New(),
  }
  for k, v := range flagDecay {
    g.decay.AddFlag(k, v)
  }
  return g
}

func (g *RaubgrafGame) valid() error {
  if g == nil {
    return fmt.Errorf("nil game object")
  }
  if g.world == nil {
    return fmt.Errorf("gameboard not created")
  }
  if g.decay == nil {
    return fmt.Errorf("flag decay not configured")
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

func (g *RaubgrafGame) clearT(t *board.Triangle) error {
  t.Add(g.decay)
  return nil
}

func (g *RaubgrafGame) clearV(v *board.Vertex) error {
  return nil
}

// mobV places FieldUnits from Vertex-dwelling Pops that Levy.
func (g *RaubgrafGame) mobV(v *board.Vertex) error {
  if v == nil {
    return nil
  }
  knights := v.GetSettlement().FilteredPopulation(pop.KnightFilter, pop.LevyFilter)
  for _, knight := range knights {
    k := knight.Mobilise()
    v.AddUnit(k)
    g.units = append(g.units, k)
    target := g.world.NodeAt(knight.GetTarget())
    k.Mobile.SetPath(graph.FindPath(v.Node, target, g.world.Distance, graph.DefaultCost))
  }

  return nil
}

// mobT places FieldUnits from Triangle-dwelling Pops that Levy.
func (g *RaubgrafGame) mobT(t *board.Triangle) error {
  if t == nil {
    return nil
  }

  mobs := t.FilteredPopulation(pop.PeasantFilter, pop.LevyFilter)
  mobs = append(mobs, t.FilteredPopulation(pop.BanditFilter, pop.FightingFilter)...)
  levies := mobilise(mobs)
  t.AddUnits(levies)
  g.units = append(g.units, levies...)
  return nil
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
  bestProd := 0
  for work := range peasants {
    currProd := foodAmount(work+1)
    peasantStr := float64(len(peasants) - (work+1))
    res := war.Result(banditStr, banditStr, peasantStr, peasantStr)
    stolen := nBandits - beatenBandits(nBandits, res)
    currProd -= stolen
    if currProd < bestProd {
      continue
    }
    bestProd = currProd
    nWorkers = work+1
  }
  for _, p := range peasants[nWorkers:] {
    p.SetAction(pop.Levy)
  }

  if bestProd < foodAmount(len(peasants)) / 2 {
    // Losing badly. Appeal for help.
    t.IncFlag(board.BeaconFlag)
  }
  // TODO: Go elsewhere if losing badly - also applies to bandits.

  return nil
}

// popsThinkV runs the Pop AI (for Vertex inhabitants) where not
// overridden by incoming commands.
func popsThinkV(v *board.Vertex) error {
  // Knights can decide to train, or to defend against bandits.
  knights := v.GetSettlement().FilteredPopulation(pop.KnightFilter, pop.AvailableFilter)
  triangles := make(map[*board.Triangle]int)
  for _, d := range board.TriDirs {
    tt := v.GetTriangle(d)
    if tt == nil {
      continue
    }
    // TODO: Also prioritise triangles that pay rent!
    triangles[tt] = tt.Value(board.BeaconFlag)
  }

  for _, k := range knights {
    var best *board.Triangle
    for tt, val := range triangles {
      if val < triangles[best] {
        continue
      }
      best = tt
    }
    if triangles[best] < 1 {
      break
    }
    triangles[best]--
    k.SetAction(pop.Levy)
    k.SetTarget(best.Node.Point)
  }
  return nil
}

// foodAmount returns the amount of food created by the given number of peasants.
func foodAmount(pc int) int {
  production := intMin(pc, maxProduction)
  for i, extra := range surplusFood {
    if i >= pc {
      break
    }
    production += extra
  }
  return production
}

// wealthAmount returns the amount of wealth the given number
// of working peasants creates.
func wealthAmount(pc int) int {
  amount := 0
  for _, threshold := range wealthProduction {
    if pc < threshold {
      break
    }
    amount++
  }
  return amount
}

// produce creates food and places it into storage.
func produce(t *board.Triangle) error {
  if t == nil {
    return nil
  }
  pc := t.CountPops(pop.PeasantFilter, pop.AvailableFilter)
  t.ClearLand(pc)
  if !t.IsFarm() {
    return nil
  }

  // TODO: Effects of weather, improvements.
  food := foodAmount(pc)
  if err := t.AddFood(food); err != nil {
    return fmt.Errorf("produce error when storing food: %w", err)
  }

  wealth := wealthAmount(pc)
  if err := t.AddWealth(wealth); err != nil {
    return fmt.Errorf("produce error when storing wealth: %w", err)
  }

  return nil
}

// movingUnits returns those units that still have movement points.
func movingUnits(cands []*war.FieldUnit) []*war.FieldUnit {
  ret := make([]*war.FieldUnit, 0, len(cands))
  for _, cc := range cands {
    if cc.GetNext() == nil {
      continue
    }
    if cc.GetMovePoints() < 1 {
      continue
    }
    ret = append(ret, cc)
  }
  return ret
}

// moveUnits processes FieldUnit movement.
func (g *RaubgrafGame) moveUnits() error {
  if err := g.valid(); err != nil {
    return err
  }
  movers := movingUnits(g.units)
  for len(movers) > 0 {
    g.dice.Shuffle(len(movers), func(i, j int) {
      movers[i], movers[j] = movers[j], movers[i]
    })

    for _, mob := range movers {
      cNode := g.world.NodeAt(mob.Point)
      if cNode != nil {
        cNode.RemoveUnit(mob)
      }
      loc := mob.GetNext()
      if loc == nil {
        continue
      }
      // TODO: Actually calculate movement cost here.
      mob.Move(1, *loc)
      // TODO: Check whether this starts any battles that
      // will prevent other units from moving.
      nNode := g.world.NodeAt(mob.Point)
      if nNode == nil {
        return fmt.Errorf("cannot find node at %s", mob.Point.String())
      }
      nNode.AddUnit(mob)
    }
    movers = movingUnits(movers)
  }
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
  if err := g.processBoard("Clear board", g.clearV, g.clearT, true); err != nil {
    return err
  }
  if err := g.processBoard("Pop thinking", popsThinkV, popsThinkT, true); err != nil {
    return err
  }
  if err := g.processBoard("Mobilise", g.mobV, g.mobT, true); err != nil {
    return err
  }
  if err := g.processTriangles("Production", produce); err != nil {
    return err
  }
  if err := g.moveUnits(); err != nil {
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
  // TODO: Demobilise.
  return nil
}
