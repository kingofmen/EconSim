// Package mobile contains a model for moving around on the board.
package mobile

import (
  "fmt"

  "gogames/util/coords"
)

type Mobile struct {
  coords.Point
  movement int
  path []coords.Point
}

// Move pays the provided movement cost and sets the Mobile's
// location, returning an error if the move is not possible.
func (m *Mobile) Move(cost int, loc coords.Point) error {
  if m == nil {
    return fmt.Errorf("attempt to move nil mobile")
  }
  if cost > m.movement {
    return fmt.Errorf("unit with %d movement cannot move distance %d", m.movement, cost)
  }
  m.movement -= cost
  m.SetLocation(loc)
  return nil
}

// GetNext return the next point on the Mobile's path.
func (m* Mobile) GetNext() *coords.Point {
  if m == nil || len(m.path) < 1 {
    return nil
  }
  return &m.path[0]
}

// ResetMove sets the Mobile's remaining movement points.
func (m *Mobile) ResetMove(move int) {
  if m == nil {
    return
  }
  m.movement = move
}

// SetLocation sets the Mobile's location to the new point,
// erasing it from the path if possible.
func (m *Mobile) SetLocation(loc coords.Point) {
  if m == nil {
    return
  }
  m.Point = loc
  if len(m.path) > 0 && loc == m.path[0] {
    m.path = m.path[1:]
  }
}

// SetPath sets the Mobile's intended path.
func (m *Mobile) SetPath(p []coords.Point) {
  if m == nil {
    return
  }
  m.path = p
}

// New returns a new Mobile.
func New() *Mobile {
  return &Mobile{
    Point: coords.New(0, 0),
    movement: 0,
  }
}
