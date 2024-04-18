package chain

import (
	"testing"

	cpb "gogames/settlers/economy/chain_proto"
)

func TestAllowed(t *testing.T) {
	cases := []struct {
		desc    string
		loc     *Location
		process *cpb.Process
		want    bool
	}{
		{
			desc: "Insufficient workers",
			loc:  &Location{},
			process: &cpb.Process{
				Key: "requires_work",
				Workers: &cpb.Workers{
					Skills: map[string]int32{"work": 1},
				},
			},
		},
		{
			desc: "Happy case",
			loc: &Location{
				pool: &cpb.Workers{
					Skills: map[string]int32{"work": 1, "unrelated": 1},
				},
			},
			process: &cpb.Process{
				Key: "requires_work",
				Workers: &cpb.Workers{
					Skills: map[string]int32{"work": 1},
				},
			},
			want: true,
		},
	}

	for _, cc := range cases {
		t.Run(cc.desc, func(t *testing.T) {
			if got := cc.loc.Allowed(cc.process); got != cc.want {
				t.Errorf("%s: Allowed(%s) => %v, want %v", cc.desc, cc.process.GetKey(), got, cc.want)
			}
		})
	}
}

func TestPlace(t *testing.T) {
	grain := &cpb.Process{
		Key: "grain",
		Workers: &cpb.Workers{
			Skills: map[string]int32{"labor": 100},
		},
		Outputs: &cpb.Goods{
			Goods: map[string]int32{"grain": 100},
		},
	}
	prayers := &cpb.Process{
		Key: "prayers",
		Workers: &cpb.Workers{
			Skills: map[string]int32{"theology": 100},
		},
		Outputs: &cpb.Goods{
			Goods: map[string]int32{"prayers": 1000},
		},
	}

	cases := []struct {
		desc      string
		loc       *Location
		processes []*cpb.Process
		want      string
	}{
		{
			desc:      "No workers",
			want:      "",
			loc:       &Location{},
			processes: []*cpb.Process{grain},
		},
		{
			desc: "Wrong workers",
			want: "",
			loc: &Location{
				pool: &cpb.Workers{
					Skills: map[string]int32{"flattery": 10},
				},
			},
			processes: []*cpb.Process{grain},
		},
		{
			desc: "One legal process",
			want: "grain",
			loc: &Location{
				pool: &cpb.Workers{
					Skills: map[string]int32{"labor": 100},
				},
			},
			processes: []*cpb.Process{grain, prayers},
		},
		{
			desc: "Default scoring",
			want: "prayers",
			loc: &Location{
				pool: &cpb.Workers{
					Skills: map[string]int32{
						"labor":    100,
						"theology": 100,
					},
				},
			},
			processes: []*cpb.Process{grain, prayers},
		},
		{
			desc: "Custom scoring",
			want: "grain",
			loc: &Location{
				pool: &cpb.Workers{
					Skills: map[string]int32{
						"labor":    100,
						"theology": 100,
					},
				},
				evaluator: ScorerFunc(func(exist, marginal *cpb.Goods) int32 {
					if marginal.GetGoods()["grain"] > 0 {
						return 100
					}
					return 0
				}),
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
