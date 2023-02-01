package engine

import (
  "fmt"

  "gogames/raubgraf/engine/board"
  "gogames/raubgraf/engine/pop"
)

var (
  // TODO: Probably this should be in a config file.
  foodProduction = []int{0, 3, 5, 6, 6, 5, 4, 3, 2, 1, 0}
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

// produceFood creates surplus food and places it into storage.
func produceFood(t *board.Triangle) error {
  if t == nil {
    return nil
  }
  pc := t.PopCount(pop.Peasant)
  t.ClearLand(pc)
  if !t.IsFarm() {
    return nil
  }
  if pc >= len(foodProduction) {
    pc = len(foodProduction)-1
  }
  surplus := pc + foodProduction[pc]
  // TODO: Effects of weather, improvements.

  if err := t.AddFood(surplus); err != nil {
    return fmt.Errorf("produceFood error when adding surplus: %w", err)
  }

  return nil
}

