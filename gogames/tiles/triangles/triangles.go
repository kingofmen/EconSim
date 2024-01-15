package triangles

import (
	"fmt"
)

// Coordinate system from https://www.boristhebrave.com/2021/05/23/triangle-grids/.
type TriPoint [3]int
type Direction int
type axis int

const (
	North Direction = iota
	NorthEast
	SouthEast
	South
	SouthWest
	NorthWest
)

const (
	aAxis axis = iota // Increase SouthWest
	bAxis             // Increase North
	cAxis             // Increase SouthEast

	root3Sixth = float64(0.28867513459)
)

var (
	steps = map[Direction]TriPoint{
		SouthEast: TriPoint{1, 0, 0},
		NorthWest: TriPoint{-1, 0, 0},
		North:     TriPoint{0, 1, 0},
		South:     TriPoint{0, -1, 0},
		SouthWest: TriPoint{0, 0, 1},
		NorthEast: TriPoint{0, 0, -1},
	}

	directions = map[TriPoint]Direction{
		TriPoint{1, 0, 0}:  SouthWest,
		TriPoint{-1, 0, 0}: NorthEast,
		TriPoint{0, 1, 0}:  North,
		TriPoint{0, -1, 0}: South,
		TriPoint{0, 0, 1}:  SouthEast,
		TriPoint{0, 0, -1}: NorthWest,
	}
)

// String returns a human-readable string suitable for debugging.
// It is not intended for display to users.
func (d Direction) String() string {
	switch d {
	case North:
		return "North    "
	case NorthEast:
		return "NorthEast"
	case SouthEast:
		return "SouthEast"
	case South:
		return "South    "
	case SouthWest:
		return "SouthWest"
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
		return SouthEast
	case SouthEast:
		return South
	case South:
		return SouthWest
	case SouthWest:
		return NorthWest
	case NorthWest:
		return North
	}
	panic(fmt.Errorf("Clockwise received unknown direction %v", d))
}

// Deasil is a synonym for Clockwise.
func (d Direction) Deasil() Direction {
	return d.Clockwise()
}

// CounterClockwise returns the direction one step counterclockwise from that provided.
func (d Direction) CounterClockwise() Direction {
	switch d {
	case North:
		return NorthWest
	case NorthWest:
		return SouthWest
	case SouthWest:
		return South
	case South:
		return SouthEast
	case SouthEast:
		return NorthEast
	case NorthEast:
		return North
	}
	panic(fmt.Errorf("CounterClockwise received unknown direction %v", d))
}

// AntiClockwise is a synonym for CounterClockwise.
func (d Direction) AntiClockwise() Direction {
	return d.CounterClockwise()
}

// Widdershins is a synonym for CounterClockwise.
func (d Direction) Widdershins() Direction {
	return d.CounterClockwise()
}

// Opposite returns the 180-degrees-turned direction.
func (d Direction) Opposite() Direction {
	switch d {
	case North:
		return South
	case NorthWest:
		return SouthEast
	case SouthWest:
		return NorthEast
	case South:
		return North
	case SouthEast:
		return NorthWest
	case NorthEast:
		return SouthWest
	}
	panic(fmt.Errorf("Opposite received unknown direction %v", d))
}

// A returns the a, first, coordinate.
func (tp TriPoint) A() int {
	return tp[aAxis]
}

// B returns the b, second, coordinate.
func (tp TriPoint) B() int {
	return tp[bAxis]
}

// C returns the c, third, coordinate.
func (tp TriPoint) C() int {
	return tp[cAxis]
}

// String returns a human-readable string suitable for debugging.
func (tp TriPoint) String() string {
	return fmt.Sprintf("(%d, %d, %d)", tp.A(), tp.B(), tp.C())
}

// Copy returns a copy of the array.
func (tp TriPoint) Copy() TriPoint {
	return TriPoint{tp[0], tp[1], tp[2]}
}

// sum returns the coordinate sum, which indicates validity and pointing.
func (tp TriPoint) sum() int {
	return tp.A() + tp.B() + tp.C()
}

// add returns the vector sum.
func (tp TriPoint) add(step TriPoint) TriPoint {
	return TriPoint{tp.A() + step.A(), tp.B() + step.B(), tp.C() + step.C()}
}

// Sum returns the vector sum.
func Sum(tps ...TriPoint) TriPoint {
	ret := TriPoint{0, 0, 0}
	for _, tp := range tps {
		ret[0] += tp.A()
		ret[1] += tp.B()
		ret[2] += tp.C()
	}
	return ret
}

// Valid returns an error if the coordinates do not
// describe a valid triangle position.
func (tp TriPoint) Valid() error {
	// a+b+c must add to one or two.
	switch tp.sum() {
	case 1:
		return nil
	case 2:
		return nil
	}
	return fmt.Errorf("invalid triangle checksum %d from coordinates %s", tp.sum(), tp)
}

// Vertex is one point of a triangle.
type Vertex struct {
	// At the map border some triangles may be nil.
	triangles [6]*Surface
}

// GetSurface returns the triangle in the given direction from the vertex,
// if any. Note that vertices have between one and six triangles.
func (v *Vertex) GetSurface(d Direction) *Surface {
	if v == nil {
		return nil
	}
	return v.triangles[d]
}

// Surface models a tile defined by three vertices.
type Surface struct {
	// Either N - SE - SW or NW - NE - S.
	vertices map[Direction]*Vertex

	// Opposite of vertices.
	neighbours map[Direction]*Surface

	// Triple coordinate.
	tripoint TriPoint
}

// New returns a new triangle with the given tri-coordinates.
func New(a, b, c int) (*Surface, error) {
	tp := TriPoint{a, b, c}
	if err := tp.Valid(); err != nil {
		return nil, err
	}

	tt := &Surface{
		vertices:   make(map[Direction]*Vertex),
		neighbours: make(map[Direction]*Surface),
		tripoint:   tp,
	}
	return tt, nil
}

// String returns a human-readable string suitable for debugging.
func (s *Surface) String() string {
	if s == nil {
		return "<nil>"
	}
	return s.tripoint.String()
}

// GetVertex returns the vertex in the given direction, if any.
func (t *Surface) GetVertex(d Direction) *Vertex {
	if t == nil {
		return nil
	}
	return t.vertices[d]
}

// GetNeighbour returns the triangle in the given direction, if any.
func (t *Surface) GetNeighbour(d Direction) *Surface {
	if t == nil {
		return nil
	}
	return t.neighbours[d]
}

// GetTriPoint returns a copy of the triangle's coordinates.
func (t *Surface) GetTriPoint() TriPoint {
	return t.tripoint.Copy()
}

// A returns the first (warp) coordinate.
func (t *Surface) A() int {
	if t == nil {
		return 0
	}
	return t.tripoint.A()
}

// B returns the second (weft) coordinate.
func (t *Surface) B() int {
	if t == nil {
		return 0
	}
	return t.tripoint.B()
}

// C returns the third (hex) coordinate.
func (t *Surface) C() int {
	if t == nil {
		return 0
	}
	return t.tripoint.C()
}

// X translates the A, B, C tripoint into Cartesian
// and returns the first coordinate of the centroid.
func (t *Surface) X() float64 {
	ret := float64(t.A())
	ret -= float64(t.C())
	ret *= 0.5
	return ret
}

// Y translates the A, B, C tripoint into Cartesian
// and returns the second coordinate of the centroid.
func (t *Surface) Y() float64 {
	ret := float64(2 * t.B())
	ret -= float64(t.A())
	ret -= float64(t.C())
	return root3Sixth * ret
}

// XY returns the Cartesian coordinates of the centroid.
func (t *Surface) XY() (float64, float64) {
	return t.X(), t.Y()
}

// Points returns true if the triangle has a point in the given direction.
func (t *Surface) Points(d Direction) bool {
	if t == nil {
		return false
	}
	sum := t.tripoint.sum()
	switch d {
	case North:
		return sum == 2
	case NorthEast:
		return sum == 1
	case SouthEast:
		return sum == 2
	case South:
		return sum == 1
	case SouthWest:
		return sum == 2
	case NorthWest:
		return sum == 1
	}
	return false
}

// Tile sets the vertices and neighbours of the surfaces.
func Tile(surfaces ...*Surface) error {
	trimap := make(map[TriPoint]*Surface)
	for _, face := range surfaces {
		if face == nil {
			continue
		}
		if _, ex := trimap[face.tripoint]; ex {
			return fmt.Errorf("duplicate tri-coordinate %s", face.tripoint)
		}
		if err := face.tripoint.Valid(); err != nil {
			return err
		}
		trimap[face.tripoint] = face
	}
	for _, face := range surfaces {
		if face == nil {
			continue
		}
		for dir, step := range steps {
			if face.neighbours[dir] != nil {
				// We already set this relationship from the other surface.
				continue
			}

			nbc := face.tripoint.add(step)
			if err := nbc.Valid(); err != nil {
				continue
			}
			oppDir := dir.Opposite()
			nb := trimap[nbc]
			if nb != nil {
				face.neighbours[dir] = nb
				nb.neighbours[oppDir] = face
			}

			setVtx := func(vtxDir, oppVtxDir Direction) {
				vtx := face.GetVertex(vtxDir)
				if vtx == nil {
					vtx = nb.GetVertex(oppVtxDir)
				}
				if vtx == nil {
					vtx = &Vertex{}
				}
				face.vertices[vtxDir] = vtx
				vtx.triangles[vtxDir.Opposite()] = face
				if nb != nil {
					nb.vertices[oppVtxDir] = vtx
					vtx.triangles[oppVtxDir.Opposite()] = nb
				}
			}
			setVtx(dir.Clockwise(), oppDir.AntiClockwise())
			setVtx(dir.AntiClockwise(), oppDir.Clockwise())
		}
	}
	return nil
}
