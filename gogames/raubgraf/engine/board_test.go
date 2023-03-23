package board

import (
	"math"
	"strings"
	"testing"

	"github.com/google/go-cmp/cmp"
	"gogames/raubgraf/engine/pop"
	"gogames/util/coords"
	"google.golang.org/protobuf/proto"
	"google.golang.org/protobuf/testing/protocmp"

	spb "gogames/raubgraf/protos/state_proto"
)

func TestNewBoard(t *testing.T) {
	type part struct {
		vc   coords.Point
		good map[Direction]bool
	}

	cases := []struct {
		desc    string
		w, h    int
		err     string
		expT    int
		full    coords.Point
		partial []part
	}{
		{
			desc: "bad width",
			w:    1,
			h:    2,
			err:  "received bad",
		},
		{
			desc: "bad height",
			w:    2,
			h:    1,
			err:  "received bad",
		},
		{
			desc: "2x2",
			w:    2,
			h:    2,
			expT: 2,
			partial: []part{
				{
					vc:   coords.Point{0, 0},
					good: map[Direction]bool{NorthEast: true},
				},
				{
					vc:   coords.Point{1, 0},
					good: map[Direction]bool{NorthWest: true, North: true},
				},
				{
					vc:   coords.Point{0, 1},
					good: map[Direction]bool{SouthEast: true, South: true},
				},
				{
					vc:   coords.Point{1, 1},
					good: map[Direction]bool{SouthWest: true},
				},
			},
		},
		{
			desc: "5x5",
			w:    5,
			h:    5,
			expT: 32,
			full: coords.Point{1, 2},
			partial: []part{
				{
					vc:   coords.Point{1, 0},
					good: map[Direction]bool{NorthEast: true, North: true, NorthWest: true},
				},
				{
					vc:   coords.Point{0, 1},
					good: map[Direction]bool{South: true, SouthEast: true, NorthEast: true, North: true},
				},
				{
					vc:   coords.Point{0, 2},
					good: map[Direction]bool{SouthEast: true, NorthEast: true},
				},
				{
					vc:   coords.Point{0, 4},
					good: map[Direction]bool{SouthEast: true},
				},
				{
					vc:   coords.Point{4, 1},
					good: map[Direction]bool{SouthWest: true, NorthWest: true},
				},
				{
					vc:   coords.Point{4, 0},
					good: map[Direction]bool{North: true, NorthWest: true},
				},
			},
		},
	}

	for _, cc := range cases {
		t.Run(cc.desc, func(t *testing.T) {
			b, err := New(cc.w, cc.h)
			if err == nil {
				if cc.err != "" {
					t.Errorf("New(%d, %d) => nil, want %q", cc.w, cc.h, cc.err)
					return
				}
			} else {
				if cc.err == "" {
					t.Errorf("New(%d, %d) => %v, want nil", cc.w, cc.h, err)
				}
				if !strings.Contains(err.Error(), cc.err) {
					t.Errorf("New(%d, %d) => %v, want %q", cc.w, cc.h, err, cc.err)
				}
				return
			}

			tCount := make(map[*Triangle]bool)
			for x := 0; x < cc.w; x++ {
				for y := 0; y < cc.h; y++ {
					vc := coords.Point{x, y}
					v1 := b.getVertex(x, y)
					if v1 == nil {
						t.Errorf("New(%d, %d) => coordinates (%d, %d) do not exist", cc.w, cc.h, x, y)
						continue
					}
					vcOffset := coords.Point{x + b.vtxOffset, y + b.vtxOffset}
					v2 := b.VertexAt(vcOffset)
					if v1 != v2 {
						t.Errorf("New(%d, %d) => coords %s do not give same vertex as offset coords %s", cc.w, cc.h, vc, vcOffset)
					}
					if v1.Point != vcOffset {
						t.Errorf("New(%d, %d) => coordinates %s (%s) have location %s", cc.w, cc.h, vc, vcOffset, v1.Point)
					}
					for _, td := range TriDirs {
						tt := v1.GetTriangle(td)
						if tt == nil {
							continue
						}
						tCount[tt] = true
						uniq := map[*Vertex]bool{}
						for _, vd := range TriDirs {
							if vtx := tt.GetVertex(vd); vtx != nil {
								uniq[vtx] = true
							}
						}
						if len(uniq) != 3 {
							t.Errorf("New(%d, %d) => vertex(%d, %d)->triangle(%s) has %d vertices, want 3.", cc.w, cc.h, x, y, td, len(uniq))
						}
					}
				}
			}
			if cc.expT != len(tCount) {
				t.Errorf("New(%d, %d) => %d triangles, expect %d", cc.w, cc.h, len(tCount), cc.expT)
			}

			if cc.full.X()+cc.full.Y() > 0 {
				v := b.getVertex(cc.full.X(), cc.full.Y())
				if v == nil {
					t.Errorf("New(%d, %d) => nil vertex (%v), expect full hex.", cc.w, cc.h, cc.full)
					return
				}
				for _, d := range TriDirs {
					if tt := v.GetTriangle(d); tt == nil {
						t.Errorf("New(%d, %d) => vertex (%v) has nil triangle %v, expect non-nil", cc.w, cc.h, cc.full, d)
					}
				}
			}

			for _, p := range cc.partial {
				v := b.getVertex(p.vc.X(), p.vc.Y())
				if v == nil {
					t.Errorf("New(%d, %d) => vertex (%v) is nil, expect partial %v", cc.w, cc.h, p.vc, p.good)
					continue
				}
				for _, d := range TriDirs {
					tt := v.GetTriangle(d)
					exist := (tt != nil)
					if p.good[d] != exist {
						t.Errorf("New(%d, %d) => vertex (%v) triangle %s is %v, expect %v", cc.w, cc.h, p.vc, d, exist, p.good[d])
					}
					if !exist {
						continue
					}
					if vtx := tt.GetVertex(d.Opposite()); vtx != v {
						t.Errorf("New(%d, %d) => vertex(%v)->triangle(%s)->vertex(%s) does not equal original vertex.", cc.w, cc.h, p.vc, d, d.Opposite())
					}
				}
			}

			upDirs := map[Direction]bool{NorthWest: true, NorthEast: true, South: true}
			dnDirs := map[Direction]bool{SouthWest: true, SouthEast: true, North: true}
			for idx, tt := range b.Triangles {
				if cand := b.TriangleAt(tt.Node.Point); cand != tt {
					t.Errorf("New(%d, %d) => TriangleAt(%v) returned %v", cc.w, cc.h, tt.Node.Point, cand.Node.Point)
				}
				nCount := make(map[*Triangle]bool)
				for _, d := range TriDirs {
					n := tt.GetNeighbour(d)
					if n == nil {
						continue
					}
					if tt.pointsUp() {
						if !upDirs[d] {
							t.Errorf("New(%d, %d) => triangle(%d) (up) has unexpected neighbour %s", cc.w, cc.h, idx, d)
						}
					} else {
						if !dnDirs[d] {
							t.Errorf("New(%d, %d) => triangle(%d) (down) has unexpected neighbour %s", cc.w, cc.h, idx, d)
						}
					}
					if nCount[n] {
						t.Errorf("New(%d, %d) => triangle(%d) has duplicate neighbour %v", cc.w, cc.h, idx, n)
					}
					nCount[n] = true
					opp := d.Opposite()
					if back := n.GetNeighbour(opp); back != tt {
						t.Errorf("New(%d, %d) => triangle(%d)->neighbour(%s)->neighbour(%s) is %v, expect original %v", cc.w, cc.h, idx, d, opp, back, tt)
					}
				}
				if ns := len(nCount); ns < 1 || ns > 3 {
					t.Errorf("New(%d, %d) => triangle(%d) has %d neighbours, expect 1-3", cc.w, cc.h, idx, ns)
				}
			}
		})
	}
}

func TestTrianglePops(t *testing.T) {
	type delta struct {
		k   pop.Kind
		add bool
	}
	cases := []struct {
		desc   string
		deltas []delta
	}{
		{
			desc: "Add and remove",
			deltas: []delta{
				{pop.Peasant, true},
				{pop.Bandit, true},
				{pop.Peasant, false},
				{pop.Bandit, true},
			},
		},
		{
			desc: "Bad removes",
			deltas: []delta{
				{pop.Peasant, false},
				{pop.Bandit, false},
			},
		},
		{
			desc: "Bad removes followed by good adds and removes",
			deltas: []delta{
				{pop.Peasant, false},
				{pop.Bandit, false},
				{pop.Peasant, true},
				{pop.Bandit, true},
				{pop.Peasant, false},
			},
		},
	}

	for _, cc := range cases {
		t.Run(cc.desc, func(t *testing.T) {
			tt := NewTriangle(0, North)
			for i, del := range cc.deltas {
				exp := tt.CountKind(del.k)
				before := exp
				if del.add {
					tt.AddPop(pop.New(del.k))
					exp++
				} else {
					rm := tt.RemoveKind(del.k)
					expK := del.k
					if before == 0 {
						expK = pop.Null
					}
					if rmk := rm.GetKind(); rmk != expK {
						t.Errorf("%s: %d RemoveKind() => %s, want %s", cc.desc, i, rmk, expK)
					}
					exp--
				}
				if before == 0 && !del.add {
					exp = 0
				}
				if after := tt.CountKind(del.k); after != exp {
					t.Errorf("%s: CountKind(%s) => %d after delta (%v), want %d", cc.desc, del.k, after, del.add, exp)
				}
			}
		})
	}
}

func TestFood(t *testing.T) {
	tt := NewTriangle(0, North)
	deltas := []struct {
		add int
		exp int
		err string
	}{
		{
			add: 0,
			exp: 0,
		},
		{
			add: 1,
			exp: 1,
		},
		{
			add: 10,
			exp: 11,
		},
		{
			add: -1,
			exp: 10,
		},
		{
			add: -100,
			exp: 0,
			err: "subtract 100",
		},
	}

	for _, d := range deltas {
		err := tt.AddFood(d.add)
		if d.err == "" {
			if err != nil {
				t.Errorf("AddFood(%d) => %v, want nil", d.add, err)
			}
		} else {
			if err == nil || !strings.Contains(err.Error(), d.err) {
				t.Errorf("AddFood(%d) => %v, want %q", d.add, err, d.err)
			}
		}
		if got := tt.CountFood(); got != d.exp {
			t.Errorf("GotFood(%d) => %d, want %d", d.add, got, d.exp)
		}
	}
}

func TestLandClearing(t *testing.T) {
	tt := &Triangle{
		overgrowth: 5,
	}
	deltas := []struct {
		pcs int
		exp int
	}{
		{
			pcs: 0,
			exp: 6,
		},
		{
			pcs: 1,
			exp: 5,
		},
		{
			pcs: 10,
			exp: 0,
		},
		{
			pcs: 0,
			exp: 1,
		},
	}

	for _, dd := range deltas {
		tt.ClearLand(dd.pcs)
		got := tt.CountForest()
		if got != dd.exp {
			t.Errorf("ClearLand(%d) => %d, want %d", dd.pcs, got, dd.exp)
		}
		if farm := tt.IsFarm(); farm != (dd.exp == 0) {
			t.Errorf("ClearLand(%d): IsFarm() => %v, expect %d overgrowth", dd.pcs, farm, dd.exp)
		}
	}
}

func TestPopFilters(t *testing.T) {
	cases := []struct {
		desc    string
		pops    []*pop.Pop
		filters []pop.Filter
		first   int
		all     []int
	}{
		{
			desc:    "No filters",
			pops:    []*pop.Pop{pop.New(pop.Peasant), pop.New(pop.Bandit)},
			filters: []pop.Filter{},
			first:   0,
			all:     []int{0, 1},
		},
		{
			desc:    "Hungry filter",
			pops:    []*pop.Pop{pop.New(pop.Peasant), pop.New(pop.Bandit)},
			filters: []pop.Filter{pop.HungryFilter},
			first:   -1,
			all:     []int{},
		},
		{
			desc:    "Peasant",
			pops:    []*pop.Pop{pop.New(pop.Peasant), pop.New(pop.Peasant), pop.New(pop.Bandit)},
			filters: []pop.Filter{pop.PeasantFilter},
			first:   0,
			all:     []int{0, 1},
		},
		{
			desc:    "Bandit",
			pops:    []*pop.Pop{pop.New(pop.Peasant), pop.New(pop.Peasant), pop.New(pop.Bandit), pop.New(pop.Bandit)},
			filters: []pop.Filter{pop.BanditFilter},
			first:   2,
			all:     []int{2, 3},
		},
	}

	for _, cc := range cases {
		t.Run(cc.desc, func(t *testing.T) {
			tt := NewTriangle(0, North)
			for _, p := range cc.pops {
				tt.AddPop(p)
			}

			first := tt.FirstPop(cc.filters...)
			if cc.first != -1 {
				if first == nil || first != cc.pops[cc.first] {
					t.Errorf("%s: FirstPop() => %v, expect pop %d", cc.desc, first, cc.first)
				}
			} else if first != nil {
				t.Errorf("%s: FirstPop() => %v, expect nil", cc.desc, first)
			}

			got := tt.FilteredPopulation(cc.filters...)
			if len(got) != len(cc.all) {
				t.Errorf("%s: FilteredPopulation() => %d pops, expected %d: %v", cc.desc, len(got), len(cc.all), cc.all)
			}
			for idx, g := range got {
				if idx >= len(cc.all) {
					break
				}
				exp := cc.pops[cc.all[idx]]
				if exp != g {
					t.Errorf("%s: FilteredPopulation()[%d] => %v, expect index %d", cc.desc, idx, got, cc.all[idx])
				}
			}
		})
	}
}

func TestDistance(t *testing.T) {
	bb, err := New(5, 5)
	if err != nil {
		t.Fatalf("Could not create board: %v", err)
	}
	cases := []struct {
		desc string
		src  coords.Point
		dst  coords.Point
		exp  float64
	}{
		{
			desc: "Two vertices",
			src:  coords.New(0+bb.vtxOffset, 0+bb.vtxOffset),
			dst:  coords.New(3+bb.vtxOffset, 3+bb.vtxOffset),
			exp:  math.Sqrt(18.0),
		},
		{
			desc: "Two triangles",
			src:  coords.New(0, 0),
			dst:  coords.New(3, 3),
			exp:  math.Sqrt(18.0),
		},
		{
			desc: "Triangle and vertex",
			src:  coords.New(0, 0),
			dst:  coords.New(3+bb.vtxOffset, 3+bb.vtxOffset),
			exp:  math.Sqrt(13.0) + 0.5,
		},
		{
			desc: "Bad vertex",
			src:  coords.New(0, 0),
			dst:  coords.New(1000, 1000),
			exp:  math.MaxFloat64,
		},
	}

	for _, cc := range cases {
		t.Run(cc.desc, func(t *testing.T) {
			got1 := bb.Distance(cc.src, cc.dst)
			got2 := bb.Distance(cc.dst, cc.src)
			if got1 != got2 {
				t.Errorf("%s: Asymmetric distances %f <=> %f", cc.desc, got1, got2)
			}
			if got1 != cc.exp {
				t.Errorf("%s: Distance(%s, %s) => %f, want %f", cc.desc, cc.src.String(), cc.dst.String(), got1, cc.exp)
			}
		})
	}
}

func TestProtoConversion(t *testing.T) {
	cases := []struct {
		desc string
		bp   *spb.Board
	}{
		{
			desc: "Basic board",
			bp: &spb.Board{
				Width:  proto.Uint32(3),
				Height: proto.Uint32(3),
				Triangles: []*spb.Triangle{
					&spb.Triangle{
						Xpos:   proto.Uint32(1),
						Ypos:   proto.Uint32(1),
						Forest: proto.Uint32(5),
					},
				},
			},
		},
	}

	for _, cc := range cases {
		t.Run(cc.desc, func(t *testing.T) {
			bb, err := FromProto(cc.bp)
			if err != nil {
				t.Errorf("%s: FromProto() => %v, want nil", cc.desc, err)
			}
			nbp, err := bb.ToProto()
			if err != nil {
				t.Errorf("%s: ToProto() => %v, want nil", cc.desc, err)
			}
			if diff := cmp.Diff(nbp, cc.bp, protocmp.Transform()); len(diff) > 0 {
				t.Errorf("%s: ToProto(FromProto()) => %s", cc.desc, diff)
			}
		})
	}
}
