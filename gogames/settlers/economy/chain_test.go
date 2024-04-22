package chain

import (
	"strings"
	"testing"

	cpb "gogames/settlers/economy/chain_proto"
)

func TestAllowed(t *testing.T) {
	cases := []struct {
		desc    string
		loc     *Location
		process *cpb.Process
		want    []*placement
	}{
		{
			desc: "Insufficient workers",
			loc: &Location{
				maxBox: 100,
			},
			process: &cpb.Process{
				Key: "requires_work",
				Levels: []*cpb.Level{
					{Workers: map[string]int32{"work": 1}},
				},
			},
		},
		{
			desc: "Happy case",
			loc: &Location{
				pool:   map[string]int32{"work": 1, "unrelated": 1},
				maxBox: 100,
			},
			process: &cpb.Process{
				Key: "requires_work",
				Levels: []*cpb.Level{
					{Workers: map[string]int32{"work": 1}},
				},
			},
			want: []*placement{
				&placement{pos: -1, lvl: 0},
			},
		},
		{
			desc: "Scalable process",
			loc: &Location{
				pool:   map[string]int32{"work": 1, "unrelated": 1},
				maxBox: 100,
				boxen: []*Work{
					{
						key: "scalable",
						assigned: []*cpb.Level{
							{Workers: map[string]int32{"work": 3}},
						},
					},
				},
			},
			process: &cpb.Process{
				Key: "scalable",
				Levels: []*cpb.Level{
					{Workers: map[string]int32{"work": 3}},
					{Workers: map[string]int32{"work": 1}},
				},
			},
			want: []*placement{
				&placement{pos: 0, lvl: 1},
			},
		},
		{
			desc: "Scalable and restartable",
			loc: &Location{
				pool:   map[string]int32{"work": 3},
				maxBox: 100,
				boxen: []*Work{
					{
						key: "scalable",
						assigned: []*cpb.Level{
							{Workers: map[string]int32{"work": 3}},
						},
					},
				},
			},
			process: &cpb.Process{
				Key: "scalable",
				Levels: []*cpb.Level{
					{Workers: map[string]int32{"work": 3}},
					{Workers: map[string]int32{"work": 1}},
				},
			},
			want: []*placement{
				&placement{pos: 0, lvl: 1},
				&placement{pos: -1, lvl: 0},
			},
		},
		{
			desc: "Box limit",
			loc: &Location{
				pool:   map[string]int32{"work": 3},
				maxBox: 1,
				boxen: []*Work{
					{
						key: "scalable",
						assigned: []*cpb.Level{
							{Workers: map[string]int32{"work": 3}},
						},
					},
				},
			},
			process: &cpb.Process{
				Key: "scalable",
				Levels: []*cpb.Level{
					{Workers: map[string]int32{"work": 3}},
					{Workers: map[string]int32{"work": 1}},
				},
			},
			want: []*placement{
				&placement{pos: 0, lvl: 1},
			},
		},
	}

	for _, cc := range cases {
		t.Run(cc.desc, func(t *testing.T) {
			got := cc.loc.Allowed(cc.process)
			if len(got) != len(cc.want) {
				t.Errorf("%s: Allowed(%s) => %v, want %v", cc.desc, cc.process.GetKey(), got, cc.want)
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
