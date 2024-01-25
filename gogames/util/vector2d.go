// Package vector2d contains a small utility library for 2d vectors.
package vector2d

import (
	"fmt"
	"math"
)

type Vector [2]float64

// New returns a new vector.
func New(x, y float64) Vector {
	return Vector{x, y}
}

// Zero returns the zero vector.
func Zero() Vector {
	return New(0, 0)
}

// UnitX returns the unit X vector.
func UnitX() Vector {
	return New(1, 0)
}

// UnitY returns the unit Y vector.
func UnitY() Vector {
	return New(0, 1)
}

// Copy returns a copy of the vector.
func (v Vector) Copy() Vector {
	return New(v.X(), v.Y())
}

// Len returns the length of the vector.
func (v Vector) Len() float64 {
	return math.Sqrt(v[0]*v[0] + v[1]*v[1])
}

// X returns the x component.
func (v Vector) X() float64 {
	return v[0]
}

// Y returns the y component.
func (v Vector) Y() float64 {
	return v[1]
}

// XY returns both components.
func (v Vector) XY() (float64, float64) {
	return v.X(), v.Y()
}

// Sum returns the vector sum in a new vector.
func Sum(vs ...Vector) Vector {
	ret := Zero()
	for _, v := range vs {
		ret.Add(v.XY())
	}
	return ret
}

func Diff(one Vector, vs ...Vector) Vector {
	ret := one.Copy()
	for _, v := range vs {
		ret.Sub(v.XY())
	}
	return ret
}

// Add adds the coordinates in-place.
func (v *Vector) Add(x, y float64) {
	v[0] += x
	v[1] += y
}

// AddInt adds the coordinates in-place.
func (v *Vector) AddInt(x, y int) {
	v[0] += float64(x)
	v[1] += float64(y)
}

// Mul multiplies the coordinates in-place.
func (v *Vector) Mul(k float64) {
	v[0] *= k
	v[1] *= k
}

// Sub subtracts the coordinates in-place.
func (v *Vector) Sub(x, y float64) {
	v[0] -= x
	v[1] -= y
}

// SubInt subtracts the coordinates in-place.
func (v *Vector) SubInt(x, y int) {
	v[0] -= float64(x)
	v[1] -= float64(y)
}

// Set sets the coordinates in-place.
func (v *Vector) Set(x, y float64) {
	v[0] = x
	v[1] = y
}

// SetInt sets the coordinates in-place.
func (v *Vector) SetInt(x, y int) {
	v[0] = float64(x)
	v[1] = float64(y)
}

// Dot returns the dot product.
func (v1 Vector) Dot(v2 Vector) float64 {
	return v1.X()*v2.X() + v1.Y()*v2.Y()
}

// String returns a human-readable debug string.
func (v Vector) String() string {
	return fmt.Sprintf("(%v, %v)", v.X(), v.Y())
}
