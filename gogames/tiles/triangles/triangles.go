package triangles

import (
	"fmt"
	"math"
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

	root3Third = float64(0.57735026919)
	root3Sixth = float64(0.28867513459)
)

var (
	// Add to Surface coordinates to get neighbour coordinates.
	steps = map[Direction]TriPoint{
		SouthEast: TriPoint{1, 0, 0},
		NorthWest: TriPoint{-1, 0, 0},
		North:     TriPoint{0, 1, 0},
		South:     TriPoint{0, -1, 0},
		SouthWest: TriPoint{0, 0, 1},
		NorthEast: TriPoint{0, 0, -1},
	}

	// Add to Surface coordinates to get vertex coordinates.
	vtxSteps = map[Direction]TriPoint{
		SouthEast: TriPoint{0, -1, -1},
		NorthWest: TriPoint{-1, 0, 0},
		North:     TriPoint{-1, 0, -1},
		South:     TriPoint{0, -1, 0},
		SouthWest: TriPoint{-1, -1, 0},
		NorthEast: TriPoint{0, 0, -1},
	}

	// Reverse of steps.
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

// Zero returns a zero-initialised TriPoint.
func Zero() TriPoint {
	return TriPoint{0, 0, 0}
}

// FromXY returns the tri-coordinates of the triangle
// containing the point. The method is to calculate the
// distance to the zero warp/weft/hex axis by taking
// the dot product with the perpendiculars, then truncate.
func FromXY(x, y float64) TriPoint {
	return TriPoint{
		int(math.Ceil(x - root3Third*y)),
		int(math.Floor(2*root3Third*y)) + 1,
		int(math.Ceil(-x - root3Third*y)),
	}
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

// X returns the Cartesian X coordinate assuming unit edges.
// Note that this works out the same for triangle centroids
// and vertices.
func (tp TriPoint) X() float64 {
	ret := float64(tp.A())
	ret -= float64(tp.C())
	ret *= 0.5
	return ret
}

// Y returns the Cartesian Y coordinate assuming unit edges.
// Note that this works out the same for triangle centroids
// and vertices.
func (tp TriPoint) Y() float64 {
	ret := float64(2 * tp.B())
	ret -= float64(tp.A())
	ret -= float64(tp.C())
	return root3Sixth * ret
}

// XY returns the Cartesian coordinates of the tripoint.
func (tp TriPoint) XY() (float64, float64) {
	return tp.X(), tp.Y()
}

// String returns a human-readable string suitable for debugging.
func (tp TriPoint) String() string {
	return fmt.Sprintf("(%d, %d, %d)", tp.A(), tp.B(), tp.C())
}

// Copy returns a copy of the array.
func (tp TriPoint) Copy() TriPoint {
	return TriPoint{tp[0], tp[1], tp[2]}
}

// Flip returns the opposite vertex coordinate.
func (tp TriPoint) Flip() TriPoint {
	f := tp.Copy()
	// Any vertex can be specified six ways,
	// as one of the surrounding faces plus
	// a vertex step. For this flip, arbitrarily
	// use the North vertex step.
	faceSteps := Diff(f, vtxSteps[North])
	// Reverse the face steps and go south instead of north.
	return Diff(vtxSteps[South], faceSteps)
}

// sum returns the coordinate sum, which indicates validity and pointing.
func (tp TriPoint) sum() int {
	return tp.A() + tp.B() + tp.C()
}

// add returns the vector sum.
func (tp TriPoint) add(step TriPoint) TriPoint {
	return TriPoint{tp.A() + step.A(), tp.B() + step.B(), tp.C() + step.C()}
}

// Diff subtracts from the first argument.
func Diff(tps ...TriPoint) TriPoint {
	if len(tps) == 0 {
		return Zero()
	}
	ret := tps[0].Copy()
	for _, tp := range tps[1:] {
		ret[0] -= tp.A()
		ret[1] -= tp.B()
		ret[2] -= tp.C()
	}
	return ret
}

// Sum returns the vector sum.
func Sum(tps ...TriPoint) TriPoint {
	ret := Zero()
	for _, tp := range tps {
		ret[0] += tp.A()
		ret[1] += tp.B()
		ret[2] += tp.C()
	}
	return ret
}

// ValidSurface returns an error if the coordinates do not
// describe a valid triangle position.
func (tp TriPoint) ValidSurface() error {
	// a+b+c must add to one or two.
	switch tp.sum() {
	case 1:
		return nil
	case 2:
		return nil
	}
	return fmt.Errorf("invalid triangle checksum %d from coordinates %s", tp.sum(), tp)
}

// Points returns true if the triangle has a point in the given direction.
func (tp TriPoint) Points(d Direction) bool {
	sum := tp.sum()
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

// Vertex is one point of a triangle.
type Vertex struct {
	// At the map border some triangles may be nil.
	triangles [6]*Surface
	// Triple coordinate. Vertices occur between triangle lanes
	// N and N+1, and we take the coordinate as N. Sum is always 0.
	tripoint TriPoint
}

// GetTriPoint returns a copy of the vertex's coordinates.
func (v *Vertex) GetTriPoint() TriPoint {
	return v.tripoint.Copy()
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

	// Triple coordinate. Sum is either 1 (south-pointing) or 2 (north-pointing).
	tripoint TriPoint
}

// New returns a new triangle with the given tri-coordinates.
func New(a, b, c int) (*Surface, error) {
	tp := TriPoint{a, b, c}
	if err := tp.ValidSurface(); err != nil {
		return nil, err
	}

	tt := &Surface{
		vertices:   make(map[Direction]*Vertex),
		neighbours: make(map[Direction]*Surface),
		tripoint:   tp,
	}
	return tt, nil
}

// GetVertices returns the non-nil vertices of the tile.
func (s *Surface) GetVertices() []*Vertex {
	if s == nil {
		return nil
	}
	verts := make([]*Vertex, 0, 3)
	for _, v := range s.vertices {
		if v != nil {
			verts = append(verts, v)
		}
	}
	return verts
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

// Neighbours returns the non-nil neighbouring triangles and their directions.
func (t *Surface) Neighbours() map[Direction]*Surface {
	if t == nil {
		return nil
	}
	nbs := make(map[Direction]*Surface)
	for dir, nb := range t.neighbours {
		if nb != nil {
			nbs[dir] = nb
		}
	}
	return nbs
}

// GetTriPoint returns a copy of the triangle's coordinates.
func (t *Surface) GetTriPoint() TriPoint {
	return t.tripoint.Copy()
}

// Points exposes the Points method of the tri-coordinates.
func (t *Surface) Points(d Direction) bool {
	return t.tripoint.Points(d)
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
	if t == nil {
		return 0
	}
	return t.tripoint.X()
}

// Y translates the A, B, C tripoint into Cartesian
// and returns the second coordinate of the centroid.
func (t *Surface) Y() float64 {
	if t == nil {
		return 0
	}
	return t.tripoint.Y()
}

// XY returns the Cartesian coordinates of the centroid.
func (t *Surface) XY() (float64, float64) {
	return t.X(), t.Y()
}

// Bounds returns the bounding rectangle of the corresponding
// Surface.
func (tp TriPoint) Bounds() (x, y, w, h float64) {
	if tp.Points(North) {
		x, h = tp.add(vtxSteps[SouthWest]).XY()
		y = tp.add(vtxSteps[North]).Y()
		h = y - h
		w = tp.add(vtxSteps[SouthEast]).X() - x
	} else {
		x, y = tp.add(vtxSteps[NorthWest]).XY()
		h = y - tp.add(vtxSteps[South]).Y()
		w = tp.add(vtxSteps[NorthEast]).X() - x
	}
	return

}

// Bounds returns the bounding rectangle.
func (t *Surface) Bounds() (x, y, w, h float64) {
	if t.Points(North) {
		x, h = t.GetVertex(SouthWest).GetTriPoint().XY()
		y = t.GetVertex(North).GetTriPoint().Y()
		h = y - h
		w = t.GetVertex(SouthEast).GetTriPoint().X() - x
	} else {
		x, y = t.GetVertex(NorthWest).GetTriPoint().XY()
		h = y - t.GetVertex(South).GetTriPoint().Y()
		w = t.GetVertex(NorthEast).GetTriPoint().X() - x
	}
	return
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
		if err := face.tripoint.ValidSurface(); err != nil {
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
			if err := nbc.ValidSurface(); err != nil {
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
					vtx = &Vertex{
						tripoint: face.tripoint.add(vtxSteps[vtxDir]),
					}
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
