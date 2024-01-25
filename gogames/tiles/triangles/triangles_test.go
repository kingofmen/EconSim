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
				&vtxLookup{0, South}:     &vtxLookup{1, SouthWest},
			},
		},
		{
			desc: "Two disjoint triangles",
			points: [][3]int{
				{0, 0, 1}, {0, 2, 0},
			},
			numVtx: 6,
		},
		{
			desc: "Full hex",
			points: [][3]int{
				{0, 0, 1}, {1, 0, 1}, {1, 0, 0}, {1, 1, 0}, {0, 1, 0}, {0, 1, 1},
			},
			numVtx: 7,
			matches: map[*vtxLookup]*vtxLookup{
				&vtxLookup{0, NorthEast}: &vtxLookup{1, North},
				&vtxLookup{1, North}:     &vtxLookup{2, NorthWest},
				&vtxLookup{2, NorthWest}: &vtxLookup{3, SouthWest},
				&vtxLookup{3, SouthWest}: &vtxLookup{4, South},
				&vtxLookup{4, South}:     &vtxLookup{5, SouthEast},
				&vtxLookup{5, SouthEast}: &vtxLookup{0, NorthEast},
				&vtxLookup{0, South}:     &vtxLookup{1, SouthWest},
				&vtxLookup{1, SouthEast}: &vtxLookup{2, South},
				&vtxLookup{2, NorthEast}: &vtxLookup{3, SouthEast},
				&vtxLookup{3, North}:     &vtxLookup{4, NorthEast},
				&vtxLookup{4, NorthWest}: &vtxLookup{5, North},
				&vtxLookup{5, SouthWest}: &vtxLookup{0, NorthWest},
			},
		},
		{
			desc: "Line of four",
			points: [][3]int{
				{0, 0, 1}, {1, 0, 1}, {1, 0, 0}, {2, 0, 0},
			},
			numVtx: 6,
			matches: map[*vtxLookup]*vtxLookup{
				&vtxLookup{0, NorthEast}: &vtxLookup{1, North},
				&vtxLookup{0, NorthEast}: &vtxLookup{2, NorthWest},
				&vtxLookup{0, South}:     &vtxLookup{1, SouthWest},
				&vtxLookup{1, North}:     &vtxLookup{2, NorthWest},
				&vtxLookup{1, SouthEast}: &vtxLookup{2, South},
				&vtxLookup{1, SouthEast}: &vtxLookup{3, SouthWest},
				&vtxLookup{2, South}:     &vtxLookup{3, SouthWest},
				&vtxLookup{2, NorthEast}: &vtxLookup{3, North},
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
				for dir, tp := range vtxSteps {
					if vtx := tile.GetVertex(dir); vtx != nil {
						vtxCount[vtx] = true
						if face := vtx.GetSurface(dir.Opposite()); face != tile {
							t.Errorf("%s: Tile %d direction %s return is %s, want %s", cc.desc, idx, dir, face, tile)
						}
						exp := tile.GetTriPoint().add(tp)
						if vtx.GetTriPoint() != exp {
							t.Errorf("%s: Tile %d direction %s has coordinates %s, want %s", cc.desc, idx, vtx.GetTriPoint(), exp)
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

func TestSum(t *testing.T) {
	cases := []struct {
		desc     string
		summands []TriPoint
		want     TriPoint
	}{
		{
			desc:     "Add two",
			summands: []TriPoint{{1, 2, 3}, {4, 5, 6}},
			want:     TriPoint{5, 7, 9},
		},
		{
			desc: "Nothing",
			want: TriPoint{0, 0, 0},
		},
		{
			desc:     "Negative numbers",
			summands: []TriPoint{{1, 2, 3}, {4, -5, 6}, {-1, 0, 4}},
			want:     TriPoint{4, -3, 13},
		},
	}

	for _, cc := range cases {
		t.Run(cc.desc, func(t *testing.T) {
			got := Sum(cc.summands...)
			if got != cc.want {
				t.Errorf("%s: Sum(%v) => %s, want %s", cc.desc, cc.summands, got, cc.want)
			}
		})
	}
}

func TestFromXY(t *testing.T) {
	cases := []struct {
		x, y float64
		want TriPoint
	}{
		{
			x:    0,
			y:    0,
			want: TriPoint{0, 1, 0},
		},
		{
			x:    0,
			y:    0.5,
			want: TriPoint{0, 1, 0},
		},
		{
			x:    0.999,
			y:    0,
			want: TriPoint{1, 1, 0},
		},
		{
			x:    1,
			y:    0,
			want: TriPoint{1, 1, -1},
		},
		{
			x:    0,
			y:    -1,
			want: TriPoint{1, -1, 1},
		},
	}

	for _, cc := range cases {
		got := FromXY(cc.x, cc.y)
		if got != cc.want {
			t.Errorf("FromXY(%v, %v) => %s, want %s", cc.x, cc.y, got, cc.want)
		}
	}
}
