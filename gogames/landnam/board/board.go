package board

import (
	//"fmt"

	//"gogames/util/logic"
	"gogames/tiles/triangles"

	brdpb "gogames/landnam/board/board_go_proto"
)

// ValidateTiles checks that the tiles have allowed tri-coordinates.
func ValidateTiles(brd *brdpb.Board) []error {
	errs := make([]error, 0, len(brd.GetTiles()))
	for _, tile := range brd.GetTiles() {
		if err := triangles.ProtoValid(tile.GetCoords()); err != nil {
			errs = append(errs, err)
		}
	}
	return errs
}
