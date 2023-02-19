// Package mobile contains a model for moving around on the board.
package mobile

import (
  "fmt"

  "gogames/util/coords"
)

type Mobile struct {
  coords.Point
  movement int
}

func (m *Mobile) Move(cost int, loc coords.Point) error {
  if m == nil {
    return fmt.Errorf("attempt to move nil mobile")
  }
  if cost > m.movement {
    return fmt.Errorf("unit with %d movement cannot move distance %d", m.movement, cost)
  }
  m.movement -= cost
  m.Point = loc
  return nil
}

func (m *Mobile) ResetMove(move int) {
  if m == nil {
    return
  }
  m.movement = move
}

func New() *Mobile {
  return &Mobile{
    Point: coords.New(0, 0),
    movement: 0,
  }
}
