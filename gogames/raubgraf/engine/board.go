package board

import (
  "fmt"

  "gogames/raubgraf/engine/econ"
  "gogames/raubgraf/engine/pop"
)

type Direction int

const (
  maxForest = 10
  
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

// Vertex is one point of a triangle; it may contain a castle or city.
type Vertex struct {
  // At the map border some triangles may be nil.
  triangles [6]*Triangle
}

// vCoord unambiguously identifies a vertex.
type vCoord struct {
  x, y int
}

// tCoord unambiguously identifies a triangle.
type tCoord struct {
  vtx vCoord
  dir Direction
}

// setT sets the triangle t in direction d.
func (v *Vertex) setT(t *Triangle, d Direction) error {
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

// GetTriangle returns the triangle in the given direction from the vertex,
// if any. Note that vertices have between one and six triangles.
func (v *Vertex) GetTriangle(d Direction) *Triangle {
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

// Triangle models a board segment defined by three vertices.
// It implements FoodStore and embeds Dwelling.
type Triangle struct {
  *pop.Dwelling
  
  // Either N - SE - SW or NW - NE - S.
  vertices [3]*Vertex

  // Opposite of vertices.
  neighbours [3]*Triangle

  // Level of forest.
  overgrowth int

  // Stored food.
  food int

  // Capital improvements made.
  improvements int

  // Points up or down?
  points Direction

  // Feudal obligations.
  contract *econ.Contract
}

// NewTriangle returns a new triangle with the provided growth and pointing.
func NewTriangle(forest int, d Direction) *Triangle {
  return &Triangle{
    Dwelling: pop.NewDwelling(),
    overgrowth: forest,
    points: d,
  }
}

// GetContract returns the rent agreement for the triangle, if any.
func (t *Triangle) GetContract() *econ.Contract {
  if t == nil {
    return nil
  }
  return t.contract
}

// SetContract sets the rent.
func (t *Triangle) SetContract(c *econ.Contract) {
  if t == nil {
    return
  }
  t.contract = c
}

// GetVertex returns the vertex in the given direction, if any.
// Note that all triangles have exactly three vertices.
func (t *Triangle) GetVertex(d Direction) *Vertex {
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
func (t *Triangle) GetNeighbour(d Direction) *Triangle {
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
func (t *Triangle) pointsUp() bool {
  if t == nil {
    return false
  }
  return t.points == North
}
  
// AddFood adds the provided amount of food to the farm,
// which may be negative. An error is returned if the farm
// would end up with negative food.
func (t *Triangle) AddFood(a int) error {
  if t == nil {
    return nil
  }
  t.food += a
  if t.food < 0 {
    err := fmt.Errorf("attempt to subtract %d food from stockpile %d", -a, t.food - a)
    t.food = 0
    return err
  }

  return nil
}

// CountFood returns the amount of stored food in the triangle.
func (t* Triangle) CountFood() int {
  if t == nil {
    return 0
  }
  return t.food
}

// CountForest returns the overgrowth level of the triangle.
func (t *Triangle) CountForest() int {
  if t == nil {
    return 0
  }
  return t.overgrowth  
}

// IsFarm returns true if the triangle can be farmed.
func (t *Triangle) IsFarm() bool {
  if t == nil {
    return false
  }
  return t.overgrowth == 0
}

// ClearLand removes overgrowth equal to pc, or if it is zero, increases overgrowth.
func (t *Triangle) ClearLand(pc int) {
  if t == nil {
    return
  }
  if pc < 1 {
    t.overgrowth++
    if t.overgrowth > maxForest {
      t.overgrowth = maxForest
    }
    return
  }

  t.overgrowth -= pc
  if t.overgrowth <= 0 {
    t.overgrowth = 0
  }
}

// Board is a container for the game surface, consisting of triangles and vertices.
type Board struct {
  vertices [][]*Vertex
  Triangles []*Triangle
}

// GetVertex returns the vertex at (x, y).
func (b *Board) GetVertex(x, y int) *Vertex {
  if b == nil {
    return nil
  }
  if x < 0 || y < 0 {
    return nil
  }
  if x >= len(b.vertices) {
    return nil
  }
  if y >= len(b.vertices[x]) {
    return nil
  }
  return b.vertices[x][y]
}

// VertexAt returns the vertex at the coordinates vc.
func (b *Board) VertexAt(vc *vCoord) *Vertex {
  return b.GetVertex(vc.x, vc.y)
}

// neighbour returns the neighbour of the vertex at (x, y)
// in the direction d, if it exists.
func (b *Board) neighbour(x, y int, d Direction) *Vertex {
  if b == nil {
    return nil
  }
  e := y%2
  w := 1 - e
  switch d {
  case North:
    return b.GetVertex(x+0, y+2)
  case NorthEast:
    return b.GetVertex(x+e, y+1)
  case East:
    return b.GetVertex(x+1, y+0)
  case SouthEast:
    return b.GetVertex(x+e, y-1)
  case South:
    return b.GetVertex(x+0, y-2)
  case SouthWest:
    return b.GetVertex(x-w, y-1)
  case West:
    return b.GetVertex(x-1, y+0)
  case NorthWest:
    return b.GetVertex(x-w, y+1)
  }
  return nil
}

// needN returns true if the vertex at (x, y) should construct a triangle to its North.
func needN(width, height, x, y int) bool {
  if y >= height-1 {
    return false
  }
  if x == 0 {
    return y%2 == 1
  }
  if x == width-1 {
    return y%2 == 0
  }
  return true
}

// needNE returns true if the vertex at (x, y) should construct a triangle to its NorthEast.
func needNE(width, height, x, y int) bool {
  if y >= height-1 {
    return false
  }
  if x >= width-1 {
    return false
  }
  return true
}


// New creates and returns a Board with width*height vertices
// and the attendant triangles.
func New(width, height int) (*Board, error) {
  if width < 2 || height < 2 {
    return nil, fmt.Errorf("NewBoard received bad (width, height) = (%d, %d)", width, height)
  }
  board := &Board{
    vertices: make([][]*Vertex, width),
    Triangles: make([]*Triangle, 0, width*height),
  }
  for x := range board.vertices {
    board.vertices[x] = make([]*Vertex, height)
    for y := range board.vertices[x] {
      board.vertices[x][y] = &Vertex{}
    }
  }

  for y := 0; y < height; y++ {
    for x := 0; x < width; x++ {
      // For each row, create northwards triangles.
      vtx := board.vertices[x][y]
      // Create North triangle if needed.
      if needN(width, height, x, y) {
        t := NewTriangle(0, South)
        board.Triangles = append(board.Triangles, t)
        if err := vtx.setT(t, North); err != nil {
          return nil, fmt.Errorf("Error constructing vertex (%d, %d) N: %w", x, y, err)
        }
        if nw := board.neighbour(x, y, NorthWest); nw != nil {
          if err := nw.setT(t, SouthEast); err != nil {
            return nil, fmt.Errorf("Error constructing vertex (%d, %d) N-NW: %w", x, y, err)
          }
        }
        if nw := board.neighbour(x, y, NorthEast); nw != nil {
          if err := nw.setT(t, SouthWest); err != nil {
            return nil, fmt.Errorf("Error constructing vertex (%d, %d) N-NE: %w", x, y, err)
          }
        }
        if nwt := vtx.GetTriangle(NorthWest); nwt != nil {
          nwt.neighbours[1] = t
          t.neighbours[2] = nwt
        }
      }
      // Create NorthEast triangle if needed.
      if needNE(width, height, x, y) {
        t := NewTriangle(0, North)
        board.Triangles = append(board.Triangles, t)
        if err := vtx.setT(t, NorthEast); err != nil {
          return nil, fmt.Errorf("Error constructing vertex (%d, %d) NW: %w", x, y, err)
        }
        if nw := board.neighbour(x, y, NorthEast); nw != nil {
          if err := nw.setT(t, South); err != nil {
            return nil, fmt.Errorf("Error constructing vertex (%d, %d) NW-S: %w", x, y, err)
          }
        }
        if e := board.neighbour(x, y, East); e != nil {
          if err := e.setT(t, NorthWest); err != nil {
            return nil, fmt.Errorf("Error constructing vertex (%d, %d) NW-E: %w", x, y, err)
          }
        }
        if nt := vtx.GetTriangle(North); nt != nil {
          nt.neighbours[1] = t
          t.neighbours[0] = nt
        }
        if set := vtx.GetTriangle(SouthEast); set != nil {
          set.neighbours[0] = t
          t.neighbours[2] = set
        }
      }
    }
  }
  
  return board, nil
}
