package engine

import (
  "fmt"
  "testing"

  "gogames/raubgraf/engine/board"
  "gogames/raubgraf/engine/pop"
)

func TestProcessTriangles(t *testing.T) {
  tCount := make(map[*board.Triangle]bool)
  vCount := make(map[*board.Vertex]bool)
  countFunc := func(t *board.Triangle) error {
    if t == nil {
      return fmt.Errorf("nil triangle")
    }
    tCount[t] = true
    for _, d := range board.VtxDirs {
      if vtx := t.GetVertex(d); vtx != nil {
        vCount[vtx] = true
      }
    }
    return nil
  }

  b, err := board.New(2, 2)
  if err != nil {
    t.Fatalf("Could not create board: %v", err)
  }
  game := &RaubgrafGame{
    world: b,
  }

  if err := game.processTriangles("test", countFunc); err != nil {
    t.Errorf("processTriangles(countFunc) => %v, want nil", err)
  }
  if len(tCount) != 2 {
    t.Errorf("processTriangles(countFunc) => %d triangles, want 2", len(tCount))
  }
  if len(vCount) != 4 {
    t.Errorf("processTriangles(countFunc) => %d vertices, want 4", len(vCount))
  }
}


func TestProduceFood(t *testing.T) {
  cases := []struct{
    desc string
    workers int
    forest int
    production int
  } {
      {
        desc: "One peasant",
        workers: 1,
        production: 4,
      },
      {
        desc: "Too much forest",
        forest: 5,
        workers: 1,
        production: 0,
      },
      {
        desc: "No workers",
      },
      {
        desc: "Forest is cleared",
        forest: 5,
        workers: 5,
        production: 10,
      },
      {
        desc: "Subsistence",
        forest: 5,
        workers: 10,
        production: 10,
      },
    }

  for _, cc := range cases {
    t.Run(cc.desc, func(t *testing.T) {
      tt := board.NewTriangle(cc.forest)
      for w := 0; w < cc.workers; w++ {
        tt.AddPop(pop.New(pop.Peasant))
      }
      if err := produceFood(tt); err != nil {
        t.Errorf("%s: produceFood() => %v, want nil", cc.desc, err)
        return
      }
      if got := tt.CountFood(); got != cc.production {
        t.Errorf("%s: produceFood() => %d, want %d", cc.desc, got, cc.production)
      }
    })
  }
}