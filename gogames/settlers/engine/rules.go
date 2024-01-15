package settlers

import (
	"gogames/tiles/triangles"
)

// Rule models a requirement for a piece to be placed.
type Rule interface {
	Allow(tile *Tile, fac *Faction, vertices ...*triangles.Vertex) error
	// TODO: Edges.
}
