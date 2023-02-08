package board

import (
  "strings"
  "testing"

  "gogames/raubgraf/engine/pop"
)

func TestNewBoard(t *testing.T) {
  type part struct {
      vc *vCoord
      good map[Direction]bool
  }
  
  cases := []struct {
    desc string
    w, h int
    err string
    expT int
    full *vCoord
    partial []part
  } {
      {
        desc: "bad width",
        w: 1,
        h: 2,
        err: "received bad",
      },
      {
        desc: "bad height",
        w: 2,
        h: 1,
        err: "received bad",
      },
      {
        desc: "2x2",
        w: 2,
        h: 2,
        expT: 2,
        partial: []part{
          {
            vc: &vCoord{0, 0},
            good: map[Direction]bool{NorthEast: true},
          },
          {
            vc: &vCoord{1, 0},
            good: map[Direction]bool{NorthWest: true, North: true},
          },
          {
            vc: &vCoord{0, 1},
            good: map[Direction]bool{SouthEast: true, South: true},
          },
          {
            vc: &vCoord{1, 1},
            good: map[Direction]bool{SouthWest: true},
          },
        },
      },
      {
        desc: "5x5",
        w: 5,
        h: 5,
        expT: 32,
        full: &vCoord{1, 2},
        partial: []part{
          {
            vc: &vCoord{1, 0},
            good: map[Direction]bool{NorthEast: true, North: true, NorthWest: true},
          },
          {
            vc: &vCoord{0, 1},
            good: map[Direction]bool{South: true, SouthEast: true, NorthEast: true, North: true},
          },
          {
            vc: &vCoord{0, 2},
            good: map[Direction]bool{SouthEast: true, NorthEast: true},
          },
          {
            vc: &vCoord{0, 4},
            good: map[Direction]bool{SouthEast: true},
          },
          {
            vc: &vCoord{4, 1},
            good: map[Direction]bool{SouthWest: true, NorthWest: true},
          },
          {
            vc: &vCoord{4, 0},
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
          v := b.GetVertex(x, y)
          if v == nil {
            t.Errorf("New(%d, %d) => coordinates (%d, %d) do not exist", cc.w, cc.h, x, y)
            continue
          }
          for _, td := range TriDirs {
            tt := v.GetTriangle(td)
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

      if cc.full != nil {
        v := b.VertexAt(cc.full)
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
        v := b.VertexAt(p.vc)
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
    k pop.Kind
    add bool
  }
  cases := []struct {
    desc string
    deltas []delta
  } {
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
  tt := &Triangle{}
  deltas := []struct{
    add int
    exp int
    err string
  } {
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
  deltas := []struct{
    pcs int
    exp int
  } {
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
  cases := []struct{
    desc string
    pops []*pop.Pop
    filters []pop.Filter
    first int
    all []int
  } {
      {
        desc: "No filters",
        pops: []*pop.Pop{pop.New(pop.Peasant), pop.New(pop.Bandit)},
        filters: []pop.Filter{},
        first: 0,
        all: []int{0, 1},
      },
      {
        desc: "Hungry filter",
        pops: []*pop.Pop{pop.New(pop.Peasant), pop.New(pop.Bandit)},
        filters: []pop.Filter{pop.HungryFilter},
        first: -1,
        all: []int{},
      },
      {
        desc: "Peasant",
        pops: []*pop.Pop{pop.New(pop.Peasant), pop.New(pop.Peasant), pop.New(pop.Bandit)},
        filters: []pop.Filter{pop.PeasantFilter},
        first: 0,
        all: []int{0, 1},
      },
      {
        desc: "Bandit",
        pops: []*pop.Pop{pop.New(pop.Peasant), pop.New(pop.Peasant), pop.New(pop.Bandit), pop.New(pop.Bandit)},
        filters: []pop.Filter{pop.BanditFilter},
        first: 2,
        all: []int{2, 3},
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
