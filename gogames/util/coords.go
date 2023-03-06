package coords

import (
	"fmt"
	"math"
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

// IsNil returns true if the Point is the special nil point.
func (p Point) IsNil() bool {
	return p[0] == math.MinInt32 && p[1] == math.MinInt32
}

// String returns a human-readable debugging string;
// it is not intended to be user-visible.
func (p Point) String() string {
	if p.IsNil() {
		return "<nil>"
	}
	return fmt.Sprintf("(%d, %d)", p.X(), p.Y())
}

// New returns a new Point.
func New(x, y int) Point {
	return Point{x, y}
}

// Nil returns a Point with coordinates indicating
// that it's not meant to be used.
func Nil() Point {
	// TODO: Replace with MinInt when we have a Go version that supports that.
	return New(math.MinInt32, math.MinInt32)
}
