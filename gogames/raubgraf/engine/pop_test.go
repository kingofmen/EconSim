package pop

import (
	"testing"
)

func TestHunger(t *testing.T) {
	pp := New(Peasant)
	cases := []struct {
		food bool
		exp  int
	}{
		{
			food: false,
			exp:  1,
		},
		{
			food: false,
			exp:  2,
		},
		{
			food: false,
			exp:  3,
		},
		{
			food: true,
			exp:  0,
		},
		{
			food: false,
			exp:  1,
		},
	}

	for idx, cc := range cases {
		pp.Eat(cc.food)
		if got := pp.GetHunger(); got != cc.exp {
			t.Errorf("TestHunger(%d): GetHunger() -> %d, want %d", idx, got, cc.exp)
		}
	}
}

func TestFilters(t *testing.T) {
	cases := []struct {
		desc    string
		pp      *Pop
		filters []Filter
		exp     bool
	}{
		{
			desc:    "Empty pop, no filters",
			pp:      &Pop{},
			filters: []Filter{},
			exp:     true,
		},
		{
			desc:    "Empty pop, bandit filter",
			pp:      &Pop{},
			filters: []Filter{BanditFilter},
			exp:     false,
		},
		{
			desc: "Hungry bandit",
			pp: &Pop{
				kind:   Bandit,
				hungry: 1,
			},
			filters: []Filter{BanditFilter, HungryFilter},
			exp:     true,
		},
		{
			desc: "Hungry peasant",
			pp: &Pop{
				kind:   Peasant,
				hungry: 1,
			},
			filters: []Filter{BanditFilter, HungryFilter},
			exp:     false,
		},
		{
			desc: "Peasant, bandit filter",
			pp: &Pop{
				kind:   Peasant,
				hungry: 1,
			},
			filters: []Filter{BanditFilter},
			exp:     false,
		},
		{
			desc: "Peasant, peasant filter",
			pp: &Pop{
				kind:   Peasant,
				hungry: 1,
			},
			filters: []Filter{PeasantFilter},
			exp:     true,
		},
		{
			desc: "Hungry peasant, not-hungry filter",
			pp: &Pop{
				kind:   Peasant,
				hungry: 1,
			},
			filters: []Filter{NotHungryFilter},
			exp:     false,
		},
		{
			desc: "Peasant, not-hungry filter",
			pp: &Pop{
				kind:   Peasant,
				hungry: 0,
			},
			filters: []Filter{NotHungryFilter},
			exp:     true,
		},
		{
			desc: "Noble, KindsFilter with noble",
			pp: &Pop{
				kind: Noble,
			},
			filters: []Filter{KindsFilter(Peasant, Knight, Noble)},
			exp:     true,
		},
		{
			desc: "Noble, KindsFilter without",
			pp: &Pop{
				kind: Noble,
			},
			filters: []Filter{KindsFilter(Peasant, Knight)},
			exp:     false,
		},
	}

	for _, cc := range cases {
		t.Run(cc.desc, func(t *testing.T) {
			if got := cc.pp.Pass(cc.filters...); got != cc.exp {
				t.Errorf("%s: Pass() => %v, want %v", cc.desc, got, cc.exp)
			}
		})
	}
}
