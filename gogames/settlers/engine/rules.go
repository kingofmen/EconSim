package settlers

import (
	"fmt"

	"gogames/tiles/triangles"
)

// Rule models a requirement for a piece to be placed.
type Rule interface {
	Allow(tile *Tile, fac *Faction, vertices ...*triangles.Vertex) error
	// TODO: Edges.
}

type EmptyRule struct{}

func (r *EmptyRule) Allow(tile *Tile, fac *Faction, vertices ...*triangles.Vertex) error {
	if len(tile.facePieces) > 0 {
		return fmt.Errorf("tile is not empty")
	}
	for _, v := range vertices {
		if len(tile.vtxPieces[v]) > 0 {
			return fmt.Errorf("vertex is not empty")
		}
	}
	return nil
}
