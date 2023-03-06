package engine

import (
	"fmt"
	"testing"

	"gogames/raubgraf/engine/board"
	"gogames/raubgraf/engine/econ"
	"gogames/raubgraf/engine/pop"
	"gogames/raubgraf/engine/war"
	"gogames/util/coords"
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
	game := NewGame(b)

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
	cases := []struct {
		desc    string
		workers int
		forest  int
		food    int
		wealth  int
		busy    int
	}{
		{
			desc:    "One peasant",
			workers: 1,
			food:    4,
		},
		{
			desc:    "Too much forest",
			forest:  5,
			workers: 1,
			food:    0,
		},
		{
			desc: "No workers",
		},
		{
			desc:    "Forest is cleared",
			forest:  5,
			workers: 5,
			food:    10,
			wealth:  1,
		},
		{
			desc:    "Subsistence",
			forest:  5,
			workers: 10,
			food:    10,
			wealth:  2,
		},
		{
			desc:    "Baseline for two workers",
			workers: 2,
			food:    7,
		},
		{
			desc:    "Blacksmith",
			workers: 5,
			food:    10,
			wealth:  1,
		},
		{
			desc:    "Some busy workers",
			workers: 2,
			food:    4,
			busy:    1,
		},
		{
			desc:    "All busy",
			workers: 2,
			busy:    2,
		},
	}

	for _, cc := range cases {
		t.Run(cc.desc, func(t *testing.T) {
			tt := board.NewTriangle(cc.forest, board.North)
			for w := 0; w < cc.workers; w++ {
				p := pop.New(pop.Peasant)
				if w < cc.busy {
					p.Work()
				}
				tt.AddPop(p)
			}
			if err := produce(tt); err != nil {
				t.Errorf("%s: produce() => %v, want nil", cc.desc, err)
				return
			}
			if got := tt.CountFood(); got != cc.food {
				t.Errorf("%s: produce() => %d food, want %d", cc.desc, got, cc.food)
			}
			if got := tt.CountWealth(); got != cc.wealth {
				t.Errorf("%s: produce() => %d wealth, want %d", cc.desc, got, cc.wealth)
			}
		})
	}
}

func TestConsumeFood(t *testing.T) {
	cases := []struct {
		desc     string
		food     int
		peasants int
		bandits  int
		surplus  int
		hungry   int
	}{
		{
			desc:     "basic peasants",
			food:     5,
			peasants: 4,
			surplus:  1,
		},
		{
			desc:     "add bandits",
			food:     5,
			peasants: 4,
			bandits:  1,
			surplus:  0,
		},
		{
			desc:     "someone goes hungry",
			food:     5,
			peasants: 4,
			bandits:  2,
			surplus:  0,
			hungry:   1,
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
	cases := []struct {
		desc    string
		bandits int
		hungry  int
		food    [3]int
		exp     int
	}{
		{
			desc:    "One bandit, no food",
			bandits: 1,
			hungry:  1,
			food:    [3]int{0, 0, 0},
			exp:     1,
		},
		{
			desc:    "One bandit, one food",
			bandits: 1,
			hungry:  1,
			food:    [3]int{0, 0, 1},
			exp:     0,
		},
		{
			desc:    "Three bandits, scattered food",
			bandits: 3,
			hungry:  3,
			food:    [3]int{1, 1, 1},
			exp:     0,
		},
		{
			desc:    "Three bandits, two hungry, scattered food",
			bandits: 3,
			hungry:  2,
			food:    [3]int{1, 1, 1},
			exp:     0,
		},
		{
			desc:    "Three bandits, two food",
			bandits: 3,
			hungry:  3,
			food:    [3]int{2, 0, 0},
			exp:     1,
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
		desc   string
		pops   []pop.Kind
		hunger []int
		food   int
		change map[pop.Kind]int
	}{
		{
			desc:   "No change (hungry)",
			pops:   []pop.Kind{pop.Peasant, pop.Peasant},
			hunger: []int{1, 1},
			food:   5,
			change: map[pop.Kind]int{pop.Peasant: 0},
		},
		{
			desc:   "No change (not enough food)",
			pops:   []pop.Kind{pop.Peasant, pop.Peasant},
			hunger: []int{0, 0},
			food:   0,
			change: map[pop.Kind]int{pop.Peasant: 0},
		},
		{
			desc:   "Increase with bandits",
			pops:   []pop.Kind{pop.Bandit, pop.Peasant, pop.Peasant},
			hunger: []int{1, 1, 0},
			food:   5,
			change: map[pop.Kind]int{pop.Peasant: 1, pop.Bandit: 0},
		},
		{
			desc:   "Drastic starvation",
			pops:   []pop.Kind{pop.Peasant, pop.Peasant, pop.Bandit, pop.Bandit},
			hunger: []int{3, 1, 3, 1},
			food:   5,
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

func TestPayRent(t *testing.T) {
	cases := []struct {
		desc     string
		start    int
		transfer int
		end      int
	}{
		{
			desc:     "No food",
			start:    0,
			transfer: 1,
			end:      0,
		},
		{
			desc:     "Food",
			start:    1,
			transfer: 1,
			end:      1,
		},
	}

	for _, cc := range cases {
		t.Run(cc.desc, func(t *testing.T) {
			b, err := board.New(2, 2)
			if err != nil {
				t.Fatalf("%s: Could not create board: %v", cc.desc, err)
			}
			rich := b.Triangles[0]
			poor := b.Triangles[1]
			poor.AddFood(cc.start)
			amount := econ.NewStore()
			amount.AddFood(cc.transfer)
			poor.SetContract(econ.NewContract().WithSource(poor.Store).WithTarget(rich.Store).WithAmount(amount))
			if err := payRent(poor); err != nil {
				t.Errorf("%s: payRent() => %v, expect nil", cc.desc, err)
			}
			if f := poor.CountFood(); f != 0 {
				t.Errorf("%s: payRent() => poor triangle has %d food, expect 0", cc.desc, f)
			}
			if f := rich.CountFood(); f != cc.end {
				t.Errorf("%s payRent() => rich triangle has %d food, expect %d", cc.desc, f, cc.end)
			}
		})
	}
}

func TestFight(t *testing.T) {
	cases := []struct {
		desc   string
		pops   []*pop.Pop
		levies int
		busy   int
	}{
		{
			desc: "One bandit, no levies, raid succeeds",
			pops: []*pop.Pop{
				pop.New(pop.Bandit),
				pop.New(pop.Peasant),
				pop.New(pop.Peasant),
				pop.New(pop.Peasant),
				pop.New(pop.Peasant),
			},
			levies: 0,
			busy:   0,
		},
		{
			desc: "One bandit, full levies, raid driven off",
			pops: []*pop.Pop{
				pop.New(pop.Bandit),
				pop.New(pop.Peasant),
				pop.New(pop.Peasant),
				pop.New(pop.Peasant),
				pop.New(pop.Peasant),
			},
			levies: 4,
			busy:   1,
		},
		{
			desc: "Bandits and levies evenly matched, partial success",
			pops: []*pop.Pop{
				pop.New(pop.Bandit),
				pop.New(pop.Bandit),
				pop.New(pop.Peasant),
				pop.New(pop.Peasant),
				pop.New(pop.Peasant),
				pop.New(pop.Peasant),
			},
			levies: 2,
			busy:   1,
		},
		{
			desc: "Bandits opposed by a knight",
			pops: []*pop.Pop{
				pop.New(pop.Bandit),
				pop.New(pop.Bandit),
				pop.New(pop.Bandit),
				pop.New(pop.Bandit),
				pop.New(pop.Peasant),
				pop.New(pop.Peasant),
				pop.New(pop.Peasant),
				pop.New(pop.Knight),
			},
			busy: 3,
		},
	}

	for _, cc := range cases {
		t.Run(cc.desc, func(t *testing.T) {
			tt := board.NewTriangle(0, board.North)
			for _, p := range cc.pops {
				tt.AddPop(p)
			}
			peasants := tt.FilteredPopulation(pop.PeasantFilter)
			for idx, p := range peasants {
				if idx < cc.levies {
					p.SetAction(pop.Levy)
				}
			}
			if err := fight(tt); err != nil {
				t.Errorf("%s: fight() => %v, want nil", cc.desc, err)
			}
			got := tt.FilteredPopulation(pop.BanditFilter, pop.BusyFilter)
			if len(got) != cc.busy {
				t.Errorf("%s: fight() made %d bandits busy, expect %d", cc.desc, len(got), cc.busy)
			}
		})
	}

}

func TestTrianglePopThinking(t *testing.T) {
	cases := []struct {
		desc    string
		pops    []*pop.Pop
		forest  int
		workers int
		levy    int
		beacon  bool
	}{
		{
			desc: "No bandits, max workers",
			pops: []*pop.Pop{
				pop.New(pop.Peasant),
				pop.New(pop.Peasant),
				pop.New(pop.Peasant),
				pop.New(pop.Peasant),
				pop.New(pop.Peasant),
				pop.New(pop.Peasant),
			},
			forest:  3,
			workers: 6,
		},
		{
			desc: "One bandit, six peasants",
			pops: []*pop.Pop{
				pop.New(pop.Peasant),
				pop.New(pop.Peasant),
				pop.New(pop.Peasant),
				pop.New(pop.Peasant),
				pop.New(pop.Peasant),
				pop.New(pop.Peasant),
				pop.New(pop.Bandit),
			},
			workers: 4,
			levy:    2,
		},
		{
			desc: "One bandit, two peasants",
			pops: []*pop.Pop{
				pop.New(pop.Peasant),
				pop.New(pop.Peasant),
				pop.New(pop.Bandit),
			},
			workers: 2,
		},
		{
			desc: "Bandit horde, fighting hopeless",
			pops: []*pop.Pop{
				pop.New(pop.Peasant),
				pop.New(pop.Peasant),
				pop.New(pop.Peasant),
				pop.New(pop.Peasant),
				pop.New(pop.Peasant),
				pop.New(pop.Peasant),
				pop.New(pop.Bandit),
				pop.New(pop.Bandit),
				pop.New(pop.Bandit),
				pop.New(pop.Bandit),
				pop.New(pop.Bandit),
				pop.New(pop.Bandit),
				pop.New(pop.Bandit),
				pop.New(pop.Bandit),
				pop.New(pop.Bandit),
			},
			workers: 6,
			beacon:  true,
		},
	}

	for _, cc := range cases {
		t.Run(cc.desc, func(t *testing.T) {
			tt := board.NewTriangle(cc.forest, board.North)
			for _, p := range cc.pops {
				p.Clear()
				tt.AddPop(p)
			}
			if ps := tt.CountPops(pop.PeasantFilter); ps != cc.workers+cc.levy {
				t.Fatalf("%s: Found %d peasants after setup, expect %d workers and %d levy", cc.desc, ps, cc.workers, cc.levy)
			}
			if err := popsThinkT(tt); err != nil {
				t.Fatalf("%s: popsThinkT() => %v, want nil", cc.desc, err)
			}
			if gotL := tt.CountPops(pop.PeasantFilter, pop.FightingFilter); gotL != cc.levy {
				t.Errorf("%s: popsThinkT() gave %d levies, want %d", cc.desc, gotL, cc.levy)
			}
			if gotW := tt.CountPops(pop.PeasantFilter, pop.LabourFilter); gotW != cc.workers {
				t.Errorf("%s: popsThinkT() gave %d workers, want %d", cc.desc, gotW, cc.workers)
			}
			val := tt.Value(board.BeaconFlag)
			if lit := val > 0; lit != cc.beacon {
				t.Errorf("%s: popsThinkT() => beacon value %d, want %v", cc.desc, val, cc.beacon)
			}
		})
	}
}

func TestCleanBoard(t *testing.T) {
	cases := []struct {
		desc   string
		inputs map[string]int
		exp    map[string]int
	}{
		{
			desc:   "Known decays",
			inputs: map[string]int{board.BeaconFlag: 4},
			exp:    map[string]int{board.BeaconFlag: 3},
		},
		{
			desc:   "Known floors",
			inputs: map[string]int{board.BeaconFlag: 0},
			exp:    map[string]int{board.BeaconFlag: 0},
		},
		{
			desc:   "Random flags",
			inputs: map[string]int{"foo": 1, "bar": 2, "baz": 3},
			exp:    map[string]int{"foo": 1, "bar": 2, "baz": 3},
		},
	}

	for _, cc := range cases {
		t.Run(cc.desc, func(t *testing.T) {
			gg := NewGame(nil)
			tt := board.NewTriangle(0, board.North)
			for k, v := range cc.inputs {
				tt.AddFlag(k, v)
			}
			if err := gg.clearT(tt); err != nil {
				t.Errorf("%s: clearT() => %v, want nil", cc.desc, err)
			}
			for k, want := range cc.exp {
				if got := tt.Value(k); got != want {
					t.Errorf("%s: clearT() => {%s : %d}, want %d", cc.desc, k, got, want)
				}
			}
		})
	}
}

func TestMoveUnits(t *testing.T) {
	bb, err := board.New(3, 3)
	if err != nil {
		t.Fatalf("Could not create board: %v", err)
	}
	game := NewGame(bb)

	cases := []struct {
		desc  string
		start coords.Point
		end   coords.Point
		exp   coords.Point
	}{
		{
			desc:  "Move one unit",
			start: coords.New(0, 0),
			end:   coords.New(1, 0),
			exp:   coords.New(1, 0),
		},
	}

	for _, cc := range cases {
		t.Run(cc.desc, func(t *testing.T) {
			unit := war.NewUnit()
			unit.SetLocation(cc.start)
			unit.SetPath([]coords.Point{cc.end})
			unit.ResetMove(10)
			game.units = append(game.units, unit)
			sNode := bb.NodeAt(cc.start)
			if sNode == nil {
				t.Fatalf("%s: Could not find start node at %s", cc.desc, cc.start.String())
			}
			eNode := bb.NodeAt(cc.end)
			if eNode == nil {
				t.Fatalf("%s: Could not find end node at %s", cc.desc, cc.end.String())
			}
			if err := game.moveUnits(); err != nil {
				t.Errorf("%s: moveUnits() => %v, want nil", cc.desc, err)
			}
			if got := unit.Mobile.Point; got != cc.exp {
				t.Errorf("%s: moveUnits() moved unit to %s, want %s", cc.desc, got.String(), cc.exp.String())
			}
			if sUnits := sNode.GetUnits(); len(sUnits) != 0 {
				t.Errorf("%s: Found %d units in start node %s, expect 0", cc.desc, len(sUnits), sNode.Point.String())
			}
			if eUnits := eNode.GetUnits(); len(eUnits) != 1 {
				t.Errorf("%s: Found %d units in end node %s, expect 0", cc.desc, len(eUnits), eNode.Point.String())
			}
		})
	}
}
