package board

import (
  "fmt"

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

// Directions of triangles and vertices from a vertex.
var tDirs = []Direction{North, NorthEast, SouthEast, South, SouthWest, NorthWest}
var vDirs = []Direction{NorthWest, NorthEast, East, SouthEast, SouthWest, West}

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

type vertex struct {
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
func (v *vertex) setT(t *Triangle, d Direction) error {
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

func (v *vertex) getTriangle(d Direction) *Triangle {
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

type Triangle struct {
  // Permanent inhabitants - peasants or bandits.
  population []*pop.Pop

  // Either N - SE - SW or NW - NE - S.
  vertices [3]*vertex

  // Level of forest.
  overgrowth int

  // Surplus food available.
  food int

  // Capital improvements made.
  improvements int
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

func (t* Triangle) CountFood() int {
  if t == nil {
    return 0
  }
  return t.food
}

// PopCount returns the number of k-type Pops in the triangle.
func (t *Triangle) PopCount(k pop.Kind) int {
  if t == nil {
    return 0
  }
  count := 0
  for _, p := range t.population {
    if p.GetKind() == k {
      count++
    }
  }
  return count
}

// AddPop adds the provided pop to the population.
func (t *Triangle) AddPop(p *pop.Pop) {
  if t == nil {
    return
  }
  if p == nil {
    return
  }
  t.population = append(t.population, p)
}

func (t *Triangle) RemovePop(k pop.Kind) *pop.Pop {
  if t == nil {
    return nil
  }
  for i := len(t.population) - 1; i >= 0; i-- {
    curr := t.population[i]
    if curr.GetKind() != k {
      continue
    }
    t.population = append(t.population[:i], t.population[i+1:]...)
    return curr
  }
  return nil
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

type Board struct {
  vertices [][]*vertex
  Triangles []*Triangle
}

// GetVertex returns the vertex at (x, y).
func (b *Board) GetVertex(x, y int) *vertex {
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
func (b *Board) VertexAt(vc *vCoord) *vertex {
  return b.GetVertex(vc.x, vc.y)
}

// neighbour returns the neighbour of the vertex at (x, y)
// in the direction d, if it exists.
func (b *Board) neighbour(x, y int, d Direction) *vertex {
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
    vertices: make([][]*vertex, width),
    Triangles: make([]*Triangle, 0, width*height),
  }
  for x := range board.vertices {
    board.vertices[x] = make([]*vertex, height)
    for y := range board.vertices[x] {
      board.vertices[x][y] = &vertex{}
    }
  }

  for x, verts := range board.vertices {
    // For each row, create northwards triangles.
    for y, vertex := range verts[:len(verts)-1] {
      // Create North triangle if needed.
      if needN(width, height, x, y) {
        t := &Triangle{}
        board.Triangles = append(board.Triangles, t)
        if err := vertex.setT(t, North); err != nil {
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
      }
      // Create NorthEast triangle if needed.
      if needNE(width, height, x, y) {
        t := &Triangle{}
        board.Triangles = append(board.Triangles, t)
        if err := vertex.setT(t, NorthEast); err != nil {
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
      }
    }
  }
  
  return board, nil
}
