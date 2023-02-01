package engine

import (
  "fmt"

  "gogames/raubgraf/engine/board"
  "gogames/raubgraf/engine/pop"
)

const(
  maxProduction = 4
)

var (
  // TODO: Probably this should be in a config file.
  surplusProduction = []int{3, 2, 1}
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
  for _, t := range g.world.Triangles {
    if err := f(t); err != nil {
      return fmt.Errorf("while processing (%s) triangles: %w", desc, err)
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
  pc := t.PopCount(pop.Peasant)
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
