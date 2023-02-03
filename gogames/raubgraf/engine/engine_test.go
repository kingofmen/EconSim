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
      tt := board.NewTriangle(cc.forest, board.North)
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

func TestConsumeFood(t *testing.T) {
  cases := []struct{
    desc string
    food int
    peasants int
    bandits int
    surplus int
    hungry int
  } {
      {
        desc: "basic peasants",
        food: 5,
        peasants: 4,
        surplus: 1,
      },
      {
        desc: "add bandits",
        food: 5,
        peasants: 4,
        bandits: 1,
        surplus: 0,
      },
      {
        desc: "someone goes hungry",
        food: 5,
        peasants: 4,
        bandits: 2,
        surplus: 0,
        hungry: 1,
      },
    }

  for _, cc := range cases {
    t.Run(cc.desc, func(t *testing.T) {
      tt := board.NewTriangle(0, board.North)
      if err := tt.AddFood(cc.food); err != nil {
        t.Fatalf("%s: AddFood(%d) => %v, want nil", cc.desc, cc.food, err)
        return
      }
      for i := 0; i < cc.peasants; i++ {
        tt.AddPop(pop.New(pop.Peasant))
      }
      for i := 0; i < cc.bandits; i++ {
        tt.AddPop(pop.New(pop.Bandit))
      }
      if err := consumeLocalFood(tt); err != nil {
        t.Errorf("%s: consumeLocalFood() => %v, want nil", cc.desc, err)
      }
      if got := tt.CountFood(); got != cc.surplus {
        t.Errorf("%s: consumeLocalFood() => surplus %d, want %d", cc.desc, got, cc.surplus)
      }
      hunger := 0
      for _, pp := range tt.Population() {
        hunger += pp.GetHunger()
      }
      if hunger != cc.hungry {
        t.Errorf("%s: consumeLocalFood() => left %d hungry, want %d", cc.desc, hunger, cc.hungry)
      }
    })
  }
}

func TestBanditry(t *testing.T) {
  cases := []struct{
    desc string
    bandits int
    hungry int
    food [3]int
    exp int
  } {
      {
        desc: "One bandit, no food",
        bandits: 1,
        hungry: 1,
        food: [3]int{0, 0, 0},
        exp: 1,
      },
      {
        desc: "One bandit, one food",
        bandits: 1,
        hungry: 1,
        food: [3]int{0, 0, 1},
        exp: 0,
      },
      {
        desc: "Three bandits, scattered food",
        bandits: 3,
        hungry: 3,
        food: [3]int{1, 1, 1},
        exp: 0,
      },
      {
        desc: "Three bandits, two hungry, scattered food",
        bandits: 3,
        hungry: 2,
        food: [3]int{1, 1, 1},
        exp: 0,
      },
      {
        desc: "Three bandits, two food",
        bandits: 3,
        hungry: 3,
        food: [3]int{2, 0, 0},
        exp: 1,
      },
    }

  for _, cc := range cases {
    t.Run(cc.desc, func(t *testing.T) {
      bb, err := board.New(3, 3)
      if err != nil {
        t.Fatalf("%s: Could not construct board: %v", cc.desc, err)
      }
      cave := bb.Triangles[1]
      farm1 := cave.GetNeighbour(board.North)
      farm2 := cave.GetNeighbour(board.SouthWest)
      farm3 := cave.GetNeighbour(board.SouthEast)
      for i := 0; i < cc.bandits; i++ {
        bn := pop.New(pop.Bandit)
        bn.Eat(i >= cc.hungry)
        cave.AddPop(bn)
      }

      farm1.AddFood(cc.food[0])
      farm2.AddFood(cc.food[1])
      farm3.AddFood(cc.food[2])
      if err := banditry(cave); err != nil {
        t.Errorf("%s: banditry() => %v, want nil", cc.desc, err)
      }
      if got := cave.CountPops(pop.BanditFilter, pop.HungryFilter); got != cc.exp {
        t.Errorf("%s: banditry() left %d bandits hungry, expect %d", cc.desc, got, cc.exp)
      }
    })
  }
}

func TestDemographics(t *testing.T) {
  cases := []struct {
    desc string
    pops []pop.Kind
    hunger []int
    food int
    change map[pop.Kind]int
  } {
      {
        desc: "No change (hungry)",
        pops: []pop.Kind{pop.Peasant, pop.Peasant},
        hunger: []int{1, 1},
        food: 5,
        change: map[pop.Kind]int{pop.Peasant: 0},
      },
      {
        desc: "No change (not enough food)",
        pops: []pop.Kind{pop.Peasant, pop.Peasant},
        hunger: []int{0, 0},
        food: 0,
        change: map[pop.Kind]int{pop.Peasant: 0},
      },
      {
        desc: "Increase with bandits",
        pops: []pop.Kind{pop.Bandit, pop.Peasant, pop.Peasant},
        hunger: []int{1, 1, 0},
        food: 5,
        change: map[pop.Kind]int{pop.Peasant: 1, pop.Bandit: 0},
      },
      {
        desc: "Drastic starvation",
        pops: []pop.Kind{pop.Peasant, pop.Peasant, pop.Bandit, pop.Bandit},
        hunger: []int{3, 1, 3, 1},
        food: 5,
        change: map[pop.Kind]int{pop.Peasant: -1, pop.Bandit: -1},
      },
    }

  for _, cc := range cases {
    if len(cc.pops) != len(cc.hunger) {
      t.Fatalf("%s: Bad test case, %v mismatches %v", cc.desc, cc.pops, cc.hunger)
    }
    tt := board.NewTriangle(0, board.North)
    if err := tt.AddFood(cc.food); err != nil {
      t.Fatalf("%s: AddFood() => %v, require nil", cc.desc, err)
    }
    start := make(map[pop.Kind]int)
    for idx, k := range cc.pops {
      start[k]++
      pp := pop.New(k)
      for i := 0; i < cc.hunger[idx]; i++ {
        pp.Eat(false)
      }
      tt.AddPop(pp)
    }
    if err := demographics(tt); err != nil {
      t.Errorf("%s: demographics() => %v, want nil", cc.desc, err)
    }
    for k, s := range start {
      exp := s + cc.change[k]
      got := tt.CountKind(k)
      if exp != got {
        t.Errorf("%s: demographics() => %d %ss, want %d", cc.desc, got, k, exp)
      }
    }
  }
}
