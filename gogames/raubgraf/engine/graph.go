// Package graph provides a graph for moving units around.
package graph

import(
  "gogames/raubgraf/engine/war"
  "gogames/util/coords"
)

type Node struct {
  // Node coordinates.
  coords.Point

  // Units currently in the node.
  units []*war.FieldUnit

  // Neighbouring nodes for pathfinding.
  neighbours []*Node
}

// New returns a new Node.
func New() *Node {
  return &Node{
    Point: coords.Nil(),
  }
}

// AddUnits adds the provided units to the node.
func (n *Node) AddUnits(ul []*war.FieldUnit) {
  for _, u := range ul {
    n.AddUnit(u)
  }
}

// AddUnit adds the provided unit to the node.
func (n *Node) AddUnit(u *war.FieldUnit) {
  if n == nil {
    return
  }
  u.Mobile.SetLocation(n.Point)
  n.units = append(n.units, u)
}

// GetUnits returns a slice of units in the Node.
func (n *Node) GetUnits() []*war.FieldUnit {
  if n == nil {
    return nil
  }
  ret := make([]*war.FieldUnit, 0, len(n.units))
  return append(ret, n.units...)
}
