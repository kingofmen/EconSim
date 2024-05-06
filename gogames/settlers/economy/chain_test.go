package chain

import (
	"fmt"
	"strings"
	"testing"

	cpb "gogames/settlers/economy/chain_proto"
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

func makeString(cs ...*combo) string {
	strs := make([]string, 0, len(cs))
	for _, c := range cs {
		strs = append(strs, c.debugString())
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

func TestWebValid(t *testing.T) {
	cases := []struct {
		desc string
		web  *cpb.Web
		want string
	}{
		{
			desc: "Nil",
			want: "at least one node",
		},
		{
			desc: "No nodes",
			web:  &cpb.Web{},
			want: "at least one node",
		},
		{
			desc: "No levels",
			web: &cpb.Web{
				Nodes: []*cpb.Process{
					{Key: "invalid"},
				},
			},
			want: "at least one level",
		},
		{
			desc: "Empty key",
			web: &cpb.Web{
				Nodes: []*cpb.Process{
					{
						Levels: []*cpb.Level{
							{Workers: map[string]int32{"work": 3}},
						},
					},
				},
			},
			want: "non-empty",
		},
		{
			desc: "Second empty key",
			web: &cpb.Web{
				Nodes: []*cpb.Process{
					{
						Key: "blah",
						Levels: []*cpb.Level{
							{Workers: map[string]int32{"work": 3}},
						},
					},
					{
						Levels: []*cpb.Level{
							{Workers: map[string]int32{"work": 3}},
						},
					},
				},
			},
			want: "non-empty",
		},
		{
			desc: "Duplicate key",
			web: &cpb.Web{
				Nodes: []*cpb.Process{
					{
						Key: "blah",
						Levels: []*cpb.Level{
							{Workers: map[string]int32{"work": 3}},
						},
					},
					{
						Key: "blah",
						Levels: []*cpb.Level{
							{Workers: map[string]int32{"work": 3}},
						},
					},
				},
			},
			want: "must be unique",
		},
		{
			desc: "Uneven levels",
			web: &cpb.Web{
				Nodes: []*cpb.Process{
					{
						Key: "scalable",
						Levels: []*cpb.Level{
							{Workers: map[string]int32{"work": 3}},
							{Workers: map[string]int32{"work": 1}},
						},
					},
					{
						Key: "single",
						Levels: []*cpb.Level{
							{Workers: map[string]int32{"work": 3}},
						},
					},
				},
			},
			want: "same number of levels",
		},
		{
			desc: "Bad transport",
			web: &cpb.Web{
				Nodes: []*cpb.Process{
					{
						Key: "scalable",
						Levels: []*cpb.Level{
							{Workers: map[string]int32{"work": 3}},
							{Workers: map[string]int32{"work": 1}},
						},
					},
					{
						Key: "single",
						Levels: []*cpb.Level{
							{Workers: map[string]int32{"work": 3}},
							{Workers: map[string]int32{"work": 3}},
						},
					},
				},
				Transport: []*cpb.Level{
					{Workers: map[string]int32{"work": 3}},
				},
			},
			want: "transport and production levels",
		},
		{
			desc: "Happy case",
			web: &cpb.Web{
				Nodes: []*cpb.Process{
					{
						Key: "scalable",
						Levels: []*cpb.Level{
							{Workers: map[string]int32{"work": 3}},
							{Workers: map[string]int32{"work": 1}},
						},
					},
					{
						Key: "single",
						Levels: []*cpb.Level{
							{Workers: map[string]int32{"work": 3}},
							{Workers: map[string]int32{"work": 3}},
						},
					},
				},
				Transport: []*cpb.Level{
					{Workers: map[string]int32{"work": 3}},
					{Workers: map[string]int32{"work": 3}},
				},
			},
		},
	}

	for _, cc := range cases {
		t.Run(cc.desc, func(t *testing.T) {
			err := Valid(cc.web)
			if err == nil {
				if len(cc.want) > 0 {
					t.Errorf("%s: Valid() => nil, want %q", cc.desc, cc.want)
				}
				return
			}
			if !strings.Contains(err.Error(), cc.want) {
				t.Errorf("%s: Valid() => %v, want %q", cc.desc, err, cc.want)
			}
		})
	}
}

func TestGenerate(t *testing.T) {
	subsist1 := &cpb.Process{
		Key: "subsist_1",
		Levels: []*cpb.Level{
			{
				Workers: map[string]int32{"labor": 100},
			},
			{
				Workers: map[string]int32{"labor": 100},
			},
		},
	}
	dig := &cpb.Process{
		Key: "dig_ore",
		Levels: []*cpb.Level{
			{
				Workers: map[string]int32{"labor": 100},
			},
			{
				Workers: map[string]int32{"labor": 100},
			},
		},
	}
	smelt := &cpb.Process{
		Key: "smelt_iron",
		Levels: []*cpb.Level{
			{
				Workers: map[string]int32{"skilled": 100},
			},
			{
				Workers: map[string]int32{"skilled": 100},
			},
		},
	}
	hammer := &cpb.Process{
		Key: "make_weapons",
		Levels: []*cpb.Level{
			{
				Workers: map[string]int32{"smith": 100},
			},
			{
				Workers: map[string]int32{"smith": 100},
			},
		},
	}

	subsist := &cpb.Web{
		Key:   "subsist",
		Nodes: []*cpb.Process{subsist1},
	}
	forge := &cpb.Web{
		Key:   "forge",
		Nodes: []*cpb.Process{dig, smelt, hammer},
	}

	loc1 := &Location{
		maxBox: 10,
		pool: map[string]int32{
			"labor": 1000,
		},
	}
	loc2 := &Location{
		maxBox: 10,
		pool: map[string]int32{
			"labor": 1000,
		},
	}
	loc3 := &Location{
		maxBox: 10,
		pool: map[string]int32{
			"labor": 900,
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
	}
	loc5 := &Location{
		maxBox: 10,
		pool: map[string]int32{
			"skilled": 1000,
		},
	}
	loc6 := &Location{
		maxBox: 10,
		pool: map[string]int32{
			"smith": 1000,
		},
	}
	loc1.network = []*Location{loc2, loc3}
	loc2.network = []*Location{loc1, loc3}
	loc3.network = []*Location{loc1, loc2, loc4}
	loc4.network = []*Location{loc3, loc5}
	loc5.network = []*Location{loc4, loc6}
	loc6.network = []*Location{loc5}

	cases := []struct {
		desc      string
		web       *cpb.Web
		locations []*Location
		want      []*combo
	}{
		{
			desc: "Nil web",
		},
		{
			desc: "No locations",
			web:  subsist,
		},
		{
			desc:      "Single process single location",
			web:       subsist,
			locations: []*Location{loc1},
			want: []*combo{
				{
					key:   subsist.GetKey(),
					procs: []*placement{{loc: loc1, prc: subsist1, pos: kNewBox, lvl: 0}},
				},
			},
		},
		{
			desc:      "Single process many locations",
			web:       subsist,
			locations: []*Location{loc1, loc2, loc3},
			want: []*combo{
				{
					key:   subsist.GetKey(),
					procs: []*placement{{loc: loc1, prc: subsist1, pos: kNewBox, lvl: 0}},
				},
				{
					key:   subsist.GetKey(),
					procs: []*placement{{loc: loc2, prc: subsist1, pos: kNewBox, lvl: 0}},
				},
				{
					key:   subsist.GetKey(),
					procs: []*placement{{loc: loc3, prc: subsist1, pos: 0, lvl: 1}},
				},
				{
					key:   subsist.GetKey(),
					procs: []*placement{{loc: loc3, prc: subsist1, pos: kNewBox, lvl: 0}},
				},
			},
		},
		{
			desc:      "Complex process sad case",
			web:       forge,
			locations: []*Location{loc1, loc2, loc3},
		},
		{
			desc:      "Complex process happy case",
			web:       forge,
			locations: []*Location{loc1, loc2, loc3, loc4, loc5, loc6},
			want: []*combo{
				{
					key: forge.GetKey(),
					procs: []*placement{
						{loc: loc4, prc: dig, pos: kNewBox, lvl: 0},
						{loc: loc5, prc: smelt, pos: kNewBox, lvl: 0},
						{loc: loc6, prc: hammer, pos: kNewBox, lvl: 0},
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
					t.Errorf("%s: generated[%d] => %d combos, want %d (%s vs %s)", cc.desc, idx, ng, nw, gpls.debugString(), wpls.debugString())
				}
			}
		})
	}
}
