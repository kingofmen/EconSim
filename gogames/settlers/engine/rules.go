package settlers

import (
	"gogames/tiles/triangles"
)

// Rule models a requirement for a piece to be placed.
type Rule interface {
	Triangle(tri *triangles.Surface, fac *Faction) error
	Vertex(vtx *triangles.Vertex, fac *Faction) error
	// TODO: Edge.
}
