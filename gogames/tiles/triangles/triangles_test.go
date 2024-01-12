package triangles

import (
	"testing"
)

func TestTiling(t *testing.T) {
	type vtxLookup struct {
		tIdx int
		dir  Direction
	}
	cases := []struct {
		desc    string
		points  [][3]int
		numVtx  int
		matches map[*vtxLookup]*vtxLookup
	}{
		{
			desc: "Two triangles",
			points: [][3]int{
				{0, 0, 1}, {1, 0, 1},
			},
			numVtx: 4,
			matches: map[*vtxLookup]*vtxLookup{
				&vtxLookup{0, NorthEast}: &vtxLookup{1, North},
			},
		},
	}

	for _, cc := range cases {
		t.Run(cc.desc, func(t *testing.T) {
			tiles := make([]*Surface, 0, len(cc.points))
			for _, pt := range cc.points {
				surface, err := New(pt[0], pt[1], pt[2])
				if err != nil {
					t.Errorf("%s: New(%v) => %v, want nil", cc.desc, cc.points)
				}
				tiles = append(tiles, surface)
			}
			if err := Tile(tiles...); err != nil {
				t.Errorf("%s: Tile() => %v, want nil")
			}
			vtxCount := make(map[*Vertex]bool)
			for idx, tile := range tiles {
				for dir := range steps {
					if vtx := tile.GetVertex(dir); vtx != nil {
						vtxCount[vtx] = true
						if face := vtx.GetSurface(dir.Opposite()); face != tile {
							t.Errorf("%s: Tile %d direction %s return is %s, want %s", cc.desc, idx, dir, face, tile)
						}
					}
				}
			}
			if len(vtxCount) != cc.numVtx {
				t.Errorf("%s: Tile() => %d unique vertices, want %d", cc.desc, len(vtxCount), cc.numVtx)
			}
			for one, two := range cc.matches {
				vtx1 := tiles[one.tIdx].GetVertex(one.dir)
				vtx2 := tiles[two.tIdx].GetVertex(two.dir)
				if vtx1 != vtx2 {
					t.Errorf("%s: %s->%s does not match %s->%s", cc.desc, tiles[one.tIdx], one.dir, tiles[two.tIdx], two.dir)
				}
			}
		})
	}
}
