// Package settlers defines the core gameplay loop.
package settlers

import (
	"gogames/tiles/triangles"
)

// Faction models a tribe or nation.
type Faction struct {
	// Number of people not assigned to a specific settlement.
	Freemen int64
}

// Rule models a requirement for a piece to be placed.
type Rule interface {
	Triangle(tri *triangles.Surface, fac *Faction) error
	Vertex(vtx *triangles.Vertex, fac *Faction) error
	// TODO: Edge.
}
