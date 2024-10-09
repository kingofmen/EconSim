package board

import (
	"strings"
	"testing"

	//"github.com/google/go-cmp/cmp"
	//"google.golang.org/protobuf/encoding/prototext"
	//"google.golang.org/protobuf/testing/protocmp"

	brdpb "gogames/landnam/board/board_go_proto"
	tripb "gogames/tiles/triangles/triangles_go_proto"
	//lpb "gogames/util/logic_go_proto"
)

func TestValidation(t *testing.T) {
	cases := []struct {
		desc  string
		board *brdpb.Board
		want  []string
	}{
		{
			desc: "Happy case",
			board: &brdpb.Board{
				Tiles: []*brdpb.Tile{
					&brdpb.Tile{Coords: &tripb.TriPoint{A: 1, B: 1, C: -1}},
					&brdpb.Tile{Coords: &tripb.TriPoint{A: 1, B: 0, C: 0}},
					&brdpb.Tile{Coords: &tripb.TriPoint{A: 0, B: 2, C: -1}},
				},
			},
		},
		{
			desc: "Bad triangles",
			board: &brdpb.Board{
				Tiles: []*brdpb.Tile{
					&brdpb.Tile{Coords: &tripb.TriPoint{A: 0, B: 0, C: 0}},
				},
			},
			want: []string{"invalid triangle coordinates"},
		},
	}

	for _, cc := range cases {
		t.Run(cc.desc, func(t *testing.T) {
			got := ValidateTiles(cc.board)
			if len(got) != len(cc.want) {
				t.Errorf("%s: ValidateTiles() => %v, want %v", cc.desc, got, cc.want)
				return
			}
			for idx, err := range got {
				if !strings.Contains(err.Error(), cc.want[idx]) {
					t.Errorf("%s: ValidateTiles() error %d is %v, want %q", cc.desc, idx, err, cc.want[idx])
				}
			}
		})
	}
}
