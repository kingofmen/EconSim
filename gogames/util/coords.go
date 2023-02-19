package coords

import (
  "fmt"
)

type Point [2]int

// X returns the point's x (first) coordinate.
func (p Point) X() int {
  return p[0]
}

// Y returns the point's y (second) coordinate.
func (p Point) Y() int {
  return p[1]
}

// String returns a human-readable debugging string;
// it is not intended to be user-visible.
func (p Point) String() string {
  return fmt.Sprintf("(%d, %d)", p.X(), p.Y())
}

func New(x, y int) Point {
  return Point{x, y}
}
