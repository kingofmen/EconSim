package board

import (
	"fmt"

	"gogames/tiles/triangles"

	brdpb "gogames/landnam/board/board_go_proto"
)

const (
	kMaxPops = 10
)

// ValidateTiles checks that the tiles are in allowed states.
func ValidateTiles(brd *brdpb.Board) []error {
	errs := make([]error, 0, len(brd.GetTiles()))
	for _, tile := range brd.GetTiles() {
		tp := tile.GetCoords()
		if err := triangles.ProtoValid(tp); err != nil {
			errs = append(errs, err)
		}
		if np := len(tile.GetPops()); np > kMaxPops {
			errs = append(errs, fmt.Errorf("tile %s has %d pops, max is %d", triangles.ProtoString(tp), np, kMaxPops))
		}
	}
	return errs
}
