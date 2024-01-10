package board

import (
	"fmt"
)

type Direction int

const (
	North Direction = iota
	NorthEast
	East
	SouthEast
	South
	SouthWest
	West
	NorthWest
)

// Directions of triangles from a vertex or triangle.
var TriDirs = []Direction{North, NorthEast, SouthEast, South, SouthWest, NorthWest}

// Directions of vertices from a vertex or triangle.
var VtxDirs = []Direction{NorthWest, NorthEast, East, SouthEast, SouthWest, West}

// String returns a human-readable string suitable for debugging.
// It is not intended for display to users.
func (d Direction) String() string {
	switch d {
	case North:
		return "North"
	case NorthEast:
		return "NorthEast"
	case East:
		return "East"
	case SouthEast:
		return "SouthEast"
	case South:
		return "South"
	case SouthWest:
		return "SouthWest"
	case West:
		return "West"
	case NorthWest:
		return "NorthWest"
	}
	return "BadDirection"
}

// Clockwise returns the direction one step clockwise from that provided.
func (d Direction) Clockwise() Direction {
	switch d {
	case North:
		return NorthEast
	case NorthEast:
		return East
	case East:
		return SouthEast
	case SouthEast:
		return South
	case South:
		return SouthWest
	case SouthWest:
		return West
	case West:
		return NorthWest
	case NorthWest:
		return North
	}
	panic(fmt.Errorf("Clockwise received unknown direction %v", d))
}

// CounterClockwise returns the direction one step counterclockwise from that provided.
func (d Direction) CounterClockwise() Direction {
	switch d {
	case North:
		return NorthWest
	case NorthWest:
		return West
	case West:
		return SouthWest
	case SouthWest:
		return South
	case South:
		return SouthEast
	case SouthEast:
		return East
	case East:
		return NorthEast
	case NorthEast:
		return North
	}
	panic(fmt.Errorf("CounterClockwise received unknown direction %v", d))
}

// Opposite returns the 180-degrees-turned direction.
func (d Direction) Opposite() Direction {
	switch d {
	case North:
		return South
	case NorthWest:
		return SouthEast
	case West:
		return East
	case SouthWest:
		return NorthEast
	case South:
		return North
	case SouthEast:
		return NorthWest
	case East:
		return West
	case NorthEast:
		return SouthWest
	}
	panic(fmt.Errorf("Opposite received unknown direction %v", d))
}

// Vertex is one point of a triangle.
type Vertex struct {
	// At the map border some triangles may be nil.
	triangles [6]*Surface
}

// setT sets the triangle t in direction d.
func (v *Vertex) setT(t *Surface, d Direction) error {
	if v == nil || t == nil {
		return nil
	}
	if d == East || d == West {
		return fmt.Errorf("Attempt to set vertex triangle in bad direction %v", d)
	}
	switch d {
	case North:
		v.triangles[0] = t
		t.vertices[2] = v
	case NorthEast:
		v.triangles[1] = t
		t.vertices[2] = v
	case SouthEast:
		v.triangles[2] = t
		t.vertices[0] = v
	case South:
		v.triangles[3] = t
		t.vertices[0] = v
	case SouthWest:
		v.triangles[4] = t
		t.vertices[1] = v
	case NorthWest:
		v.triangles[5] = t
		t.vertices[1] = v
	}
	return nil
}

// GetSurface returns the triangle in the given direction from the vertex,
// if any. Note that vertices have between one and six triangles.
func (v *Vertex) GetSurface(d Direction) *Surface {
	if v == nil {
		return nil
	}
	switch d {
	case North:
		return v.triangles[0]
	case NorthEast:
		return v.triangles[1]
	case SouthEast:
		return v.triangles[2]
	case South:
		return v.triangles[3]
	case SouthWest:
		return v.triangles[4]
	case NorthWest:
		return v.triangles[5]
	}
	return nil
}

// Surface models a tile defined by three vertices.
type Surface struct {
	// Either N - SE - SW or NW - NE - S.
	vertices [3]*Vertex

	// Opposite of vertices.
	neighbours [3]*Surface

	// Points up or down?
	points Direction
}

// New returns a new triangle with the given pointing.
func New(d Direction) *Surface {
	tt := &Surface{
		vertices:   [3]*Vertex{nil, nil, nil},
		neighbours: [3]*Surface{nil, nil, nil},
		points:     d,
	}
	return tt
}

// GetVertex returns the vertex in the given direction, if any.
// Note that all triangles have exactly three vertices.
func (t *Surface) GetVertex(d Direction) *Vertex {
	if t == nil {
		return nil
	}

	if t.pointsUp() {
		switch d {
		case North:
			return t.vertices[0]
		case SouthEast:
			return t.vertices[1]
		case SouthWest:
			return t.vertices[2]
		}
		return nil
	}

	switch d {
	case NorthWest:
		return t.vertices[0]
	case NorthEast:
		return t.vertices[1]
	case South:
		return t.vertices[2]
	}
	return nil
}

// GetNeighbour returns the triangle in the given direction, if any.
// All triangles have at least one and at most three neighbours.
func (t *Surface) GetNeighbour(d Direction) *Surface {
	if t == nil {
		return nil
	}

	if t.pointsUp() {
		switch d {
		case NorthWest:
			return t.neighbours[0]
		case NorthEast:
			return t.neighbours[1]
		case South:
			return t.neighbours[2]
		}
		return nil
	}

	switch d {
	case North:
		return t.neighbours[0]
	case SouthEast:
		return t.neighbours[1]
	case SouthWest:
		return t.neighbours[2]
	}
	return nil
}

// pointsUp returns true if the triangle is north-pointing.
func (t *Surface) pointsUp() bool {
	if t == nil {
		return false
	}
	return t.points == North
}
