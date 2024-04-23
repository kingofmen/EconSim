package chain

import (
	"fmt"
	"strings"
	"testing"

	cpb "gogames/settlers/economy/chain_proto"
)

func stringize(plcs []*placement) string {
	ret := "["
	for _, pl := range plcs {
		ret = fmt.Sprintf("%s {loc: %p prc: %s pos: %d lvl: %d}", ret, pl.loc, pl.prc.GetKey(), pl.pos, pl.lvl)
	}
	return ret + "]"
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
