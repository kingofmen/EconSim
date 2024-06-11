package settlers

import (
	"maps"
	"testing"

	"gogames/tiles/triangles"

	cpb "gogames/settlers/economy/chain_proto"
	conpb "gogames/settlers/economy/consumption_proto"
)

func TestTickWork(t *testing.T) {
	workerTemplate := &Template{
		key: "workers",
		shape: Shape{
			faces: []Face{
				Face{
					pos:      triangles.Zero(),
					occupied: true,
				},
			},
		},
	}
	cases := []struct {
		desc    string
		size    int
		place   []triangles.TriPoint
		want    map[string]int32
		cons    map[string]int32
		webs    []*cpb.Web
		buckets []*conpb.Bucket
	}{
		{
			desc:  "Work gets done at all",
			size:  1,
			place: []triangles.TriPoint{triangles.TriPoint{1, 0, 0}},
			webs: []*cpb.Web{
				&cpb.Web{
					Key: "trivial",
					Nodes: []*cpb.Process{
						&cpb.Process{
							Key: "trivial",
							Levels: []*cpb.Level{
								&cpb.Level{
									Workers: map[string]int32{"labour": int32(100)},
									Outputs: map[string]int32{"result": int32(100)},
								},
							},
						},
					},
				},
			},
			buckets: []*conpb.Bucket{
				&conpb.Bucket{
					Key: "trivial",
					Stuff: map[string]int32{
						"result": 50,
					},
				},
			},
			want: map[string]int32{
				"result": int32(100),
			},
			cons: map[string]int32{
				"result": int32(50),
			},
		},
	}

	for _, cc := range cases {
		board, err := NewHex(cc.size)
		if err != nil {
			t.Fatalf("%s: Could not create board: %v", cc.desc, err)
		}
		for _, pos := range cc.place {
			if errs := board.Place(NewPlacement(pos, workerTemplate)); len(errs) > 0 {
				t.Fatalf("Could not place worker template: %v", errs)
			}
		}
		if err := board.Tick(NewTick().WithWebs(cc.webs).WithBuckets(cc.buckets)); err != nil {
			t.Errorf("%s: Tick() => %v, want nil", cc.desc, err)
		}

		for _, pos := range cc.place {
			tile := board.GetTile(pos)
			if tile == nil {
				t.Fatalf("%s: No tile at %s", cc.desc, pos)
			}
			if !maps.Equal(cc.want, tile.Location.Produced) {
				t.Errorf("%s: Tick() produced => %v, want %v (%v)", cc.desc, tile.Location.Produced, cc.want, tile.Location.Reasons)
			}
			if !maps.Equal(cc.cons, tile.Location.Consumed) {
				t.Errorf("%s: Tick() consumed => %v, want %v (%v)", cc.desc, tile.Location.Consumed, cc.cons, tile.Location.Reasons)
			}
		}
	}
}
