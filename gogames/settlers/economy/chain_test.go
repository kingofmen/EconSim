package chain

import (
	"fmt"
	"maps"
	"strings"
	"testing"

	cpb "gogames/settlers/economy/chain_proto"
)

const (
	kDigProc     = "dig_ore"
	kSmeltProc   = "smelt_iron"
	kForgeProc   = "make_weapons"
	kForgeWeb    = "forge"
	kSubsistProc = "subsist1"
	kSubsistWeb  = "subsist"
)

func stringize(plcs ...[]*placement) string {
	if len(plcs) == 0 {
		return "<nil>"
	}
	ret := ""
	for _, pl := range plcs {
		ret += "\n[\n"
		for _, p := range pl {
			ret += fmt.Sprintf(" {loc: %p prc: %s pos: %d lvl: %d}\n", p.loc, p.prc.GetKey(), p.pos, p.lvl)
		}
		ret += "]"
	}
	return ret
}

func makeString(cs ...*Combo) string {
	strs := make([]string, 0, len(cs))
	for _, c := range cs {
		strs = append(strs, c.DebugString())
	}
	return strings.Join(strs, "\n")
}

func TestAllowed(t *testing.T) {
	location := &Location{}
	needWork := &cpb.Process{
		Key: "requires_work",
		Levels: []*cpb.Level{
			{Workers: map[string]int32{"work": 1}},
		},
	}
	needLabour := &cpb.Process{
		Key: "requires_labour",
		Levels: []*cpb.Level{
			{Workers: map[string]int32{"labour": 1}},
		},
	}
	scalable := &cpb.Process{
		Key: "scalable",
		Levels: []*cpb.Level{
			{Workers: map[string]int32{"work": 3}},
			{Workers: map[string]int32{"work": 2}},
			{Workers: map[string]int32{"work": 1}},
		},
	}

	cases := []struct {
		desc    string
		max     int
		pool    map[string]int32
		boxen   []*Work
		process *cpb.Process
		prev    []*placement
		want    []*placement
	}{
		{
			desc: "Nil",
		},
		{
			desc: "Insufficient workers",
			process: &cpb.Process{
				Key: "requires_work",
				Levels: []*cpb.Level{
					{Workers: map[string]int32{"work": 1}},
				},
			},
		},
		{
			desc:    "Happy case",
			pool:    map[string]int32{"work": 1, "unrelated": 1},
			process: needWork,
			want: []*placement{
				&placement{loc: location, prc: needWork, pos: -1, lvl: 0},
			},
			prev: []*placement{
				&placement{loc: &Location{}, prc: scalable, pos: -1, lvl: 0},
			},
		},
		{
			desc: "Scalable process",
			pool: map[string]int32{"work": 2, "unrelated": 1},
			boxen: []*Work{
				{
					key: "scalable",
					assigned: []*cpb.Level{
						{Workers: map[string]int32{"work": 3}},
					},
				},
			},
			process: scalable,
			want: []*placement{
				&placement{loc: location, prc: scalable, pos: 0, lvl: 1},
			},
		},
		{
			desc: "Scalable but previously scaled",
			pool: map[string]int32{"work": 2, "unrelated": 1},
			boxen: []*Work{
				{
					key: "scalable",
					assigned: []*cpb.Level{
						{Workers: map[string]int32{"work": 3}},
					},
				},
			},
			process: scalable,
			prev: []*placement{
				&placement{loc: location, prc: scalable, pos: 0, lvl: 1},
			},
		},
		{
			desc: "Scalable and restartable",
			pool: map[string]int32{"work": 3},
			boxen: []*Work{
				{
					key: "scalable",
					assigned: []*cpb.Level{
						{Workers: map[string]int32{"work": 3}},
					},
				},
			},
			process: scalable,
			want: []*placement{
				&placement{loc: location, prc: scalable, pos: 0, lvl: 1},
				&placement{loc: location, prc: scalable, pos: kNewBox, lvl: 0},
			},
		},
		{
			desc: "Scalable and restartable but previously restarted",
			pool: map[string]int32{"work": 5},
			boxen: []*Work{
				{
					key: "scalable",
					assigned: []*cpb.Level{
						{Workers: map[string]int32{"work": 3}},
					},
				},
			},
			process: scalable,
			want: []*placement{
				&placement{loc: location, prc: scalable, pos: 0, lvl: 1},
			},
			prev: []*placement{
				&placement{loc: location, prc: scalable, pos: kNewBox, lvl: 0},
			},
		},
		{
			desc: "Box limit",
			pool: map[string]int32{"work": 3},
			max:  1,
			boxen: []*Work{
				{
					key: "scalable",
					assigned: []*cpb.Level{
						{Workers: map[string]int32{"work": 3}},
					},
				},
			},
			process: scalable,
			want: []*placement{
				&placement{loc: location, prc: scalable, pos: 0, lvl: 1},
			},
		},
		{
			desc: "Box limit from prevs",
			pool: map[string]int32{"work": 3},
			max:  2,
			boxen: []*Work{
				{
					key: "scalable",
					assigned: []*cpb.Level{
						{Workers: map[string]int32{"work": 3}},
					},
				},
			},
			process: scalable,
			want: []*placement{
				&placement{loc: location, prc: scalable, pos: 0, lvl: 1},
			},
			prev: []*placement{
				&placement{loc: location, prc: needLabour, pos: kNewBox, lvl: 0},
			},
		},
	}

	for _, cc := range cases {
		t.Run(cc.desc, func(t *testing.T) {
			location.pool = cc.pool
			location.boxen = cc.boxen
			location.maxBox = 100
			if cc.max > 0 {
				location.maxBox = cc.max
			}
			got := location.Allowed(cc.process, cc.prev...)
			if len(got) != len(cc.want) {
				t.Errorf("%s: Allowed(%s) => %s, want %s", cc.desc, cc.process.GetKey(), stringize(got), stringize(cc.want))
				return
			}
			for i, g := range got {
				if *g != *cc.want[i] {
					t.Errorf("%s: Allowed(%s) %d => %+v, want %+v", cc.desc, cc.process.GetKey(), i, g, cc.want[i])
				}
			}
		})
	}
}

func TestPlace(t *testing.T) {
	grain := &cpb.Process{
		Key: "grain",
		Levels: []*cpb.Level{
			{
				Workers: map[string]int32{"labor": 100},
				Outputs: map[string]int32{"grain": 100},
			},
			{
				Workers: map[string]int32{"labor": 100},
				Outputs: map[string]int32{"grain": 200},
			},
		},
	}
	prayers := &cpb.Process{
		Key: "prayers",
		Levels: []*cpb.Level{
			{
				Workers: map[string]int32{"theology": 100},
				Outputs: map[string]int32{"prayers": 1000},
			},
			{
				Workers: map[string]int32{"theology": 100},
				Outputs: map[string]int32{"prayers": 100},
			},
		},
	}

	cases := []struct {
		desc      string
		loc       *Location
		processes []*cpb.Process
		want      string
	}{
		{
			desc: "No workers",
			want: "",
			loc: &Location{
				maxBox: 100,
			},
			processes: []*cpb.Process{grain},
		},
		{
			desc: "Wrong workers",
			want: "",
			loc: &Location{
				pool:   map[string]int32{"flattery": 10},
				maxBox: 100,
			},
			processes: []*cpb.Process{grain},
		},
		{
			desc: "One legal process",
			want: "grain",
			loc: &Location{
				pool:   map[string]int32{"labor": 100},
				maxBox: 100,
			},
			processes: []*cpb.Process{grain, prayers},
		},
		{
			desc: "Default scoring",
			want: "prayers",
			loc: &Location{
				pool: map[string]int32{
					"labor":    100,
					"theology": 100,
				},
				maxBox: 100,
			},
			processes: []*cpb.Process{grain, prayers},
		},
		{
			desc: "Custom scoring",
			want: "grain",
			loc: &Location{
				pool: map[string]int32{
					"labor":    100,
					"theology": 100,
				},
				maxBox: 100,
				evaluator: ScorerFunc(func(exist, marginal map[string]int32) int32 {
					if marginal["grain"] > 0 {
						return 100
					}
					return 0
				}),
			},
			processes: []*cpb.Process{grain, prayers},
		},
		{
			desc: "Scalable",
			want: "grain",
			loc: &Location{
				pool: map[string]int32{
					"labor":    100,
					"theology": 100,
				},
				maxBox: 2,
				boxen: []*Work{
					{
						key: "grain",
						assigned: []*cpb.Level{
							{Workers: map[string]int32{"grain": 100}},
						},
					},
					{
						key: "prayers",
						assigned: []*cpb.Level{
							{Workers: map[string]int32{"theology": 100}},
						},
					},
				},
			},
			processes: []*cpb.Process{grain, prayers},
		},
		{
			desc: "Scalable but new is better",
			want: "prayers",
			loc: &Location{
				maxBox: 2,
				pool: map[string]int32{
					"labor":    100,
					"theology": 100,
				},
				boxen: []*Work{
					{
						key: "grain",
						assigned: []*cpb.Level{
							{Workers: map[string]int32{"grain": 100}},
						},
					},
				},
			},
			processes: []*cpb.Process{grain, prayers},
		},
	}

	for _, cc := range cases {
		t.Run(cc.desc, func(t *testing.T) {
			if got := cc.loc.Place(cc.processes); got != cc.want {
				t.Errorf("%s: Place() => %s, want %s", cc.desc, got, cc.want)
			}
		})
	}
}

func makeWebs() (map[string]*cpb.Process, map[string]*cpb.Web) {
	procs := make(map[string]*cpb.Process)
	webs := make(map[string]*cpb.Web)
	procs[kSubsistProc] = &cpb.Process{
		Key: "subsist_1",
		Levels: []*cpb.Level{
			{
				Workers: map[string]int32{"labor": 100},
				Outputs: map[string]int32{"food": 100},
			},
			{
				Workers: map[string]int32{"labor": 100},
				Outputs: map[string]int32{"food": 100},
			},
		},
	}
	procs[kDigProc] = &cpb.Process{
		Key: kDigProc,
		Levels: []*cpb.Level{
			{
				Workers: map[string]int32{"labor": 100},
				Outputs: map[string]int32{"ore": 100},
			},
			{
				Workers: map[string]int32{"labor": 100},
				Outputs: map[string]int32{"ore": 100},
			},
		},
	}
	procs[kSmeltProc] = &cpb.Process{
		Key: kSmeltProc,
		Levels: []*cpb.Level{
			{
				Workers: map[string]int32{"skilled": 100},
				Outputs: map[string]int32{"iron": 100},
			},
			{
				Workers: map[string]int32{"skilled": 100},
				Outputs: map[string]int32{"iron": 100},
			},
		},
	}
	procs[kForgeProc] = &cpb.Process{
		Key: kForgeProc,
		Levels: []*cpb.Level{
			{
				Workers: map[string]int32{"smith": 100},
				Outputs: map[string]int32{"weapons": 100},
			},
			{
				Workers: map[string]int32{"smith": 100},
				Outputs: map[string]int32{"weapons": 100},
			},
		},
	}

	webs[kSubsistWeb] = &cpb.Web{
		Key:   kSubsistWeb,
		Nodes: []*cpb.Process{procs[kSubsistProc]},
	}
	webs[kForgeWeb] = &cpb.Web{
		Key:   kForgeWeb,
		Nodes: []*cpb.Process{procs[kDigProc], procs[kSmeltProc], procs[kForgeProc]},
	}

	return procs, webs
}

func makeLocations() []*Location {
	loc1 := &Location{
		maxBox: 10,
		pool: map[string]int32{
			"labor": 1000,
		},
		Goods: map[string]int32{
			"loc1": 1,
		},
	}
	loc2 := &Location{
		maxBox: 10,
		pool: map[string]int32{
			"labor": 1000,
		},
		Goods: map[string]int32{
			"loc2": 1,
		},
	}
	loc3 := &Location{
		maxBox: 10,
		pool: map[string]int32{
			"labor": 900,
		},
		Goods: map[string]int32{
			"loc3": 1,
		},
		boxen: []*Work{
			{
				key: "subsist_1",
				assigned: []*cpb.Level{
					{
						Workers: map[string]int32{"labor": 100},
						Outputs: map[string]int32{"grain": 100},
					},
				},
			},
		},
	}
	loc4 := &Location{
		maxBox: 10,
		pool: map[string]int32{
			"labor": 1000,
		},
		Goods: map[string]int32{
			"loc4": 1,
		},
	}
	loc5 := &Location{
		maxBox: 10,
		pool: map[string]int32{
			"skilled": 1000,
		},
		Goods: map[string]int32{
			"loc5": 1,
		},
	}
	loc6 := &Location{
		maxBox: 10,
		pool: map[string]int32{
			"smith": 1000,
		},
		Goods: map[string]int32{
			"loc6": 1,
		},
	}
	loc1.network = []*Location{loc2, loc3}
	loc2.network = []*Location{loc1, loc3}
	loc3.network = []*Location{loc1, loc2, loc4}
	loc4.network = []*Location{loc3, loc5}
	loc5.network = []*Location{loc4, loc6}
	loc6.network = []*Location{loc5}

	return []*Location{loc1, loc2, loc3, loc4, loc5, loc6}
}

func TestGenerate(t *testing.T) {
	procs, webs := makeWebs()
	locs := makeLocations()
	cases := []struct {
		desc      string
		web       *cpb.Web
		locations []*Location
		want      []*Combo
	}{
		{
			desc: "Nil web",
		},
		{
			desc: "No locations",
			web:  webs[kSubsistWeb],
		},
		{
			desc:      "Single process single location",
			web:       webs[kSubsistWeb],
			locations: locs[:1],
			want: []*Combo{
				{
					key:   webs[kSubsistWeb].GetKey(),
					procs: []*placement{{loc: locs[0], prc: procs[kSubsistProc], pos: kNewBox, lvl: 0}},
				},
			},
		},
		{
			desc:      "Single process many locations",
			web:       webs[kSubsistWeb],
			locations: locs[:3],
			want: []*Combo{
				{
					key:   webs[kSubsistWeb].GetKey(),
					procs: []*placement{{loc: locs[0], prc: procs[kSubsistProc], pos: kNewBox, lvl: 0}},
				},
				{
					key:   webs[kSubsistWeb].GetKey(),
					procs: []*placement{{loc: locs[1], prc: procs[kSubsistProc], pos: kNewBox, lvl: 0}},
				},
				{
					key:   webs[kSubsistWeb].GetKey(),
					procs: []*placement{{loc: locs[2], prc: procs[kSubsistProc], pos: 0, lvl: 1}},
				},
				{
					key:   webs[kSubsistWeb].GetKey(),
					procs: []*placement{{loc: locs[2], prc: procs[kSubsistProc], pos: kNewBox, lvl: 0}},
				},
			},
		},
		{
			desc:      "Complex process sad case",
			web:       webs[kForgeWeb],
			locations: locs[:3],
		},
		{
			desc:      "Complex process happy case",
			web:       webs[kForgeWeb],
			locations: locs,
			want: []*Combo{
				{
					key: kForgeWeb,
					procs: []*placement{
						{loc: locs[3], prc: procs[kDigProc], pos: kNewBox, lvl: 0},
						{loc: locs[4], prc: procs[kSmeltProc], pos: kNewBox, lvl: 0},
						{loc: locs[5], prc: procs[kForgeProc], pos: kNewBox, lvl: 0},
					},
				},
			},
		},
	}

	for _, cc := range cases {
		t.Run(cc.desc, func(t *testing.T) {
			got := generate(cc.web, cc.locations)
			if ng, nw := len(got), len(cc.want); ng != nw {
				t.Errorf("%s: generated() => %d combos, want %d (%s vs %s)", cc.desc, ng, nw, makeString(got...), makeString(cc.want...))
				return
			}
			for idx, gpls := range got {
				wpls := cc.want[idx]
				if ng, nw := len(gpls.procs), len(wpls.procs); ng != nw {
					t.Errorf("%s: generated[%d] => %d combos, want %d (%s vs %s)", cc.desc, idx, ng, nw, gpls.DebugString(), wpls.DebugString())
				}
			}
		})
	}
}

func TestInstantRunoff(t *testing.T) {
	cmb1 := &Combo{key: "1"}
	cmb2 := &Combo{key: "2"}
	cmb3 := &Combo{key: "3"}
	cmb4 := &Combo{key: "4"}
	cmb5 := &Combo{key: "5"}
	cmb6 := &Combo{key: "6"}

	locs := makeLocations()
	loc1 := locs[0]
	loc2 := locs[1]
	loc3 := locs[2]
	loc4 := locs[3]
	loc5 := locs[4]
	cases := []struct {
		desc   string
		scores map[*Location]map[*Combo]int32
		want   map[*Combo]bool
	}{
		{
			desc: "Nil votes",
			want: map[*Combo]bool{nil: true},
		},
		{
			desc: "Communist election",
			scores: map[*Location]map[*Combo]int32{
				loc1: map[*Combo]int32{cmb1: int32(1)},
				loc2: map[*Combo]int32{cmb1: int32(1)},
			},
			want: map[*Combo]bool{cmb1: true},
		},
		{
			desc: "Landslide",
			scores: map[*Location]map[*Combo]int32{
				loc1: map[*Combo]int32{cmb1: int32(5), cmb2: int32(4)},
				loc2: map[*Combo]int32{cmb1: int32(5), cmb2: int32(4)},
				loc3: map[*Combo]int32{cmb1: int32(5), cmb2: int32(4)},
				loc4: map[*Combo]int32{cmb1: int32(5), cmb2: int32(4)},
				loc5: map[*Combo]int32{cmb1: int32(5), cmb2: int32(4)},
			},
			want: map[*Combo]bool{cmb1: true},
		},
		{
			desc: "Close election",
			scores: map[*Location]map[*Combo]int32{
				loc1: map[*Combo]int32{cmb2: int32(5), cmb1: int32(4)},
				loc2: map[*Combo]int32{cmb2: int32(5), cmb1: int32(4)},
				loc3: map[*Combo]int32{cmb2: int32(5), cmb1: int32(4)},
				loc4: map[*Combo]int32{cmb1: int32(5), cmb2: int32(4)},
				loc5: map[*Combo]int32{cmb1: int32(5), cmb2: int32(4)},
			},
			want: map[*Combo]bool{cmb2: true},
		},
		{
			desc: "Third-party tiebreak",
			scores: map[*Location]map[*Combo]int32{
				loc1: map[*Combo]int32{cmb2: int32(5), cmb3: int32(4), cmb1: int32(3)},
				loc2: map[*Combo]int32{cmb2: int32(5), cmb3: int32(4), cmb1: int32(3)},
				loc3: map[*Combo]int32{cmb3: int32(5), cmb1: int32(4), cmb2: int32(3)},
				loc4: map[*Combo]int32{cmb1: int32(5), cmb3: int32(4), cmb2: int32(3)},
				loc5: map[*Combo]int32{cmb1: int32(5), cmb3: int32(4), cmb2: int32(3)},
			},
			want: map[*Combo]bool{cmb1: true},
		},
		{
			desc: "Overlapping options, no clear winner",
			scores: map[*Location]map[*Combo]int32{
				loc1: map[*Combo]int32{cmb1: int32(5), cmb2: int32(4), cmb3: int32(3)},
				loc2: map[*Combo]int32{cmb2: int32(5), cmb3: int32(4), cmb4: int32(3)},
				loc3: map[*Combo]int32{cmb3: int32(5), cmb4: int32(4), cmb5: int32(3)},
				loc4: map[*Combo]int32{cmb4: int32(5), cmb5: int32(4), cmb6: int32(3)},
				loc5: map[*Combo]int32{cmb5: int32(5), cmb6: int32(4)},
			},
			want: map[*Combo]bool{cmb1: true, cmb2: true, cmb3: true, cmb4: true, cmb5: true},
		},
	}

	for _, cc := range cases {
		combos := []*Combo{cmb1, cmb2, cmb3, cmb4, cmb5, cmb6}
		got := instantRunoff(combos, cc.scores)
		if !cc.want[got] {
			t.Errorf("%s: instantRunoff() => %v, want %v", cc.desc, got, cc.want)
		}
	}
}

func TestPlaceWeb(t *testing.T) {
	procs, webs := makeWebs()
	locs := makeLocations()
	allWebs := []*cpb.Web{webs[kSubsistWeb], webs[kForgeWeb]}

	cases := []struct {
		desc string
		webs []*cpb.Web
		locs []*Location
		want *Combo
	}{
		{
			desc: "Nil inputs",
		},
		{
			desc: "No locations",
			webs: allWebs,
		},
		{
			desc: "Need skilled labour for complex web",
			webs: allWebs,
			locs: locs[:1],
			want: &Combo{
				key: kSubsistWeb,
				procs: []*placement{
					{
						loc: locs[0],
						prc: procs[kSubsistProc],
						pos: kNewBox,
						lvl: 0,
					},
				},
			},
		},
		{
			desc: "Level up where possible",
			webs: allWebs,
			locs: locs[2:3],
			want: &Combo{
				key: kSubsistWeb,
				procs: []*placement{
					{
						loc: locs[2],
						prc: procs[kSubsistProc],
						pos: 0,
						lvl: 1,
					},
				},
			},
		},
		{
			desc: "Voting",
			webs: allWebs,
			locs: locs,
			// Locations 5 and 6 cannot vote for subsistence as they
			// don't have unskilled labour, so they always form a sufficient
			// voting bloc to get kForgeWeb adapted.
			want: &Combo{
				key: kForgeWeb,
				procs: []*placement{
					{
						loc: locs[3],
						prc: procs[kDigProc],
						pos: kNewBox,
						lvl: 0,
					},
					{
						loc: locs[4],
						prc: procs[kSmeltProc],
						pos: kNewBox,
						lvl: 0,
					},
					{
						loc: locs[5],
						prc: procs[kForgeProc],
						pos: kNewBox,
						lvl: 0,
					},
				},
			},
		},
	}

	for _, cc := range cases {
		t.Run(cc.desc, func(t *testing.T) {
			got := Place(cc.webs, cc.locs)
			if gs, ws := makeString(got), makeString(cc.want); gs != ws {
				t.Errorf("%s: Place() => %s,\n want %s", cc.desc, gs, ws)
			}
		})
	}
}

func TestWork(t *testing.T) {
	cases := []struct {
		desc    string
		loc     *Location
		workers map[string]int32
		want    map[string]int32
		pool    map[string]int32
	}{
		{
			desc: "Nil location",
		},
		{
			desc:    "No work",
			loc:     &Location{},
			workers: map[string]int32{"labor": 100},
			pool:    map[string]int32{"labor": 100},
		},
		{
			desc: "Priorities",
			loc: &Location{
				Goods: make(map[string]int32),
				boxen: []*Work{
					&Work{
						assigned: []*cpb.Level{
							&cpb.Level{
								Workers: map[string]int32{"labor": 50},
								Outputs: map[string]int32{"food": 50},
							},
							&cpb.Level{
								Workers: map[string]int32{"labor": 40},
								Outputs: map[string]int32{"food": 30},
							},
						},
					},
					nil,
					&Work{
						assigned: []*cpb.Level{
							&cpb.Level{
								Workers: map[string]int32{"skilled": 50},
								Outputs: map[string]int32{"goods": 50},
							},
						},
					},
					&Work{
						assigned: []*cpb.Level{
							&cpb.Level{
								Workers: map[string]int32{"skilled": 30},
								Outputs: map[string]int32{"goods": 30},
							},
							&cpb.Level{
								Workers: map[string]int32{"skilled": 30},
								Outputs: map[string]int32{"goods": 30},
							},
						},
					},
				},
			},
			workers: map[string]int32{"labor": 100, "skilled": 100},
			pool:    map[string]int32{"labor": 10, "skilled": 20},
			want:    map[string]int32{"food": 80, "goods": 80},
		},
	}

	for _, cc := range cases {
		t.Run(cc.desc, func(t *testing.T) {
			cc.loc.MakeAvailable(cc.workers)
			var gotpool map[string]int32
			if cc.loc != nil {
				gotpool = cc.loc.pool
			}
			if !maps.Equal(gotpool, cc.pool) {
				t.Errorf("%s: MakeAvailable() => %v, want %v", cc.desc, cc.loc.pool, cc.pool)
			}
			cc.loc.Work()
			var gotgoods map[string]int32
			if cc.loc != nil {
				gotgoods = cc.loc.Goods
			}
			if !maps.Equal(gotgoods, cc.want) {
				t.Errorf("%s: Work() => %v, want %v", cc.desc, cc.loc.Goods, cc.want)
			}
		})
	}
}
