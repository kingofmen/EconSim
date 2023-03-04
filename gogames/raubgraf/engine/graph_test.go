package graph

import(
  "testing"
)

func TestFindPath(t *testing.T) {
  cases := []struct {
    desc string
    nodes []*Node
    edges map[int][]int
    start int
    end int
    exp []int
  } {
      {
        desc: "Straight line",
        nodes: []*Node{
          New(0, 0),
          New(0, 1),
          New(0, 2),
        },
        edges: map[int][]int{0: []int{1}, 1: []int{2}},
        start: 0,
        end: 2,
        exp: []int{1, 2},
      },
      {
        desc: "Two paths",
        nodes: []*Node{
          New(0, 0),
          New(1, 2),
          New(1, 1),
          New(0, 1),
        },
        edges: map[int][]int{0: []int{1, 3}, 2: []int{1, 3}},
        start: 0,
        end: 2,
        exp: []int{3, 2},
      },
      {
        desc: "Dead end",
        nodes: []*Node{
          New(0, 0),
          New(1, 0),
          New(1, 1),
          New(1, -2),
          New(2, -1),
          New(2, -2),
        },
        edges: map[int][]int{1: []int{0, 2, 3, 4}, 5: []int{3}},
        start: 0,
        end: 5,
        exp: []int{1, 3, 5},
      },
    }

  for _, cc := range cases {
    t.Run(cc.desc, func(t *testing.T) {
      for n, nbs := range cc.edges {
        for _, nb := range nbs {
          cc.nodes[n].AddNeighbour(cc.nodes[nb])
          cc.nodes[nb].AddNeighbour(cc.nodes[n])
        }
      }

      got := FindPath(cc.nodes[cc.start], cc.nodes[cc.end], DefaultDistance, DefaultCost)
      if len(got) != len(cc.exp) {
        t.Errorf("%s: FindPath() => %v, expect %v", cc.desc, got, cc.exp)
        return
      }

      for idx, pt := range got {
        if exp := cc.nodes[cc.exp[idx]].Point; exp != pt  {
          t.Errorf("%s: FindPath(%d) => %v, expect %v", cc.desc, idx, pt, exp)
        }
      }
    })
  }
}

