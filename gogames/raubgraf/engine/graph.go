// Package graph provides a graph for moving units around.
package graph

import(
  "math"

  "gogames/raubgraf/engine/war"
  "gogames/util/coords"
)

type Heuristic func(s, e coords.Point) float64
type CostCalc func(s, e coords.Point) float64

type Node struct {
  // Node coordinates.
  coords.Point

  // Units currently in the node.
  units []*war.FieldUnit

  // Neighbouring nodes for pathfinding.
  neighbours []*Node
}

func DefaultCost(s, e coords.Point) float64 {
  xdist := float64(e.X() - s.X())
  ydist := float64(e.Y() - s.Y())
  return math.Sqrt(xdist*xdist + ydist*ydist)
}

func DefaultDistance(s, e coords.Point) float64 {
  xdist := float64(e.X() - s.X())
  ydist := float64(e.Y() - s.Y())
  return math.Sqrt(xdist*xdist + ydist*ydist)
}

// New returns a new Node.
func New(x, y int) *Node {
  return &Node{
    Point: coords.New(x, y),
  }
}

// AddNeighbour adds the provided node as a neighbour.
func (n *Node) AddNeighbour(nb *Node) {
  if n == nil || nb == nil {
    return
  }
  for _, ex := range n.neighbours {
    if ex == nb {
      return
    }
  }

  n.neighbours = append(n.neighbours, nb)
}

// AddUnits adds the provided units to the node.
func (n *Node) AddUnits(ul []*war.FieldUnit) {
  if n == nil {
    return
  }
  for _, u := range ul {
    n.AddUnit(u)
  }
}

// AddUnit adds the provided unit to the node.
func (n *Node) AddUnit(u *war.FieldUnit) {
  if n == nil {
    return
  }
  // Make sure we do not double-add.
  for _, c := range n.units {
    if c == u {
      return
    }
  }
  n.units = append(n.units, u)
}

func (n *Node) RemoveUnit(u *war.FieldUnit) {
  if n == nil {
    return
  }
  idx := -1
  for i, c := range n.units {
    if c != u {
      continue
    }
    idx = i
    break
  }
  if idx < 0 {
    return
  }
  n.units = append(n.units[:idx], n.units[idx+1:]...)
}

// makePath returns the path from start to end given
// the previous mapping.
func makePath(start, end *Node, prev map[*Node]*Node) []coords.Point {
  path := []coords.Point{end.Point}
  curr := end
  for {
    curr = prev[curr]
    if curr == start || curr == nil {
      break
    }
    path = append(path, curr.Point)
  }
  for i := 0; i < len(path)/2; i++ {
    other := len(path) - i - 1
    path[i], path[other] = path[other], path[i]
  }
  return path
}

// FindPath sets the path for the mobile to get to the target,
// if there is one.
func FindPath(start, end *Node, dist Heuristic, cost CostCalc) []coords.Point {
  if start == nil || end == nil {
    return nil
  }
  if start == end {
    return nil
  }

  open := map[*Node]bool{start: true}
  paid := map[*Node]float64{start: 0}
  heur := map[*Node]float64{start: dist(start.Point, end.Point)}
  prev := make(map[*Node]*Node)
  for len(open) > 0 {
    var best *Node
    low := math.MaxFloat64
    for n := range open {
      c := heur[n] + paid[n]
      if c >= low {
        continue
      }
      best = n
      low = c
    }
    delete(open, best)
    for _, nb := range best.neighbours {
      if nb == end {
        prev[nb] = best
        return makePath(start, end, prev)
      }
      if _, ex := heur[nb]; !ex {
        heur[nb] = dist(nb.Point, end.Point)
      }
      nCost := low + cost(best.Point, nb.Point)
      if oCost, ex := paid[nb]; !ex || nCost < oCost {
        paid[nb] = nCost
        open[nb] = true
        prev[nb] = best
      }
    }
  }
  return nil
}

// GetUnits returns a slice of units in the Node.
func (n *Node) GetUnits() []*war.FieldUnit {
  if n == nil {
    return nil
  }
  ret := make([]*war.FieldUnit, 0, len(n.units))
  return append(ret, n.units...)
}
