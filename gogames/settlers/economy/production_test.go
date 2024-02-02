package production

import (
	"testing"

	prodpb "gogames/settlers/economy/production_proto"
)

func TestRequirements(t *testing.T) {
	cases := []struct {
		desc  string
		exist map[string]float64
		goods []*prodpb.Good
		want  map[string]bool
	}{
		{
			desc:  "No requirements",
			exist: map[string]float64{},
			goods: []*prodpb.Good{
				{
					Key: "one",
				},
				{
					Key: "two",
				},
			},
			want: map[string]bool{
				"one": true, "two": true,
			},
		},
		{
			desc:  "Require full bucket (fail)",
			exist: map[string]float64{"one": 0.5},
			goods: []*prodpb.Good{
				{
					Key:     "one",
					Maximum: 1.0,
				},
				{
					Key: "two",
					Prereqs: []*prodpb.Requirement{
						{
							Key:  "one",
							Kind: prodpb.Requirement_RK_FULL,
						},
					},
				},
			},
			want: map[string]bool{
				"one": true,
			},
		},
		{
			desc:  "Require full bucket (succeed)",
			exist: map[string]float64{"one": 1.0},
			goods: []*prodpb.Good{
				{
					Key:     "one",
					Maximum: 1.0,
				},
				{
					Key: "two",
					Prereqs: []*prodpb.Requirement{
						{
							Key:  "one",
							Kind: prodpb.Requirement_RK_FULL,
						},
					},
				},
			},
			want: map[string]bool{
				"one": true, "two": true,
			},
		},
		{
			desc:  "Require amount (fail)",
			exist: map[string]float64{"one": 0.5},
			goods: []*prodpb.Good{
				{
					Key: "one",
				},
				{
					Key: "two",
					Prereqs: []*prodpb.Requirement{
						{
							Key:    "one",
							Kind:   prodpb.Requirement_RK_MIN,
							Amount: 0.75,
						},
					},
				},
			},
			want: map[string]bool{
				"one": true,
			},
		},
		{
			desc:  "Require amount (succeed)",
			exist: map[string]float64{"one": 1.0},
			goods: []*prodpb.Good{
				{
					Key: "one",
				},
				{
					Key: "two",
					Prereqs: []*prodpb.Requirement{
						{
							Key:    "one",
							Kind:   prodpb.Requirement_RK_MIN,
							Amount: 0.75,
						},
					},
				},
			},
			want: map[string]bool{
				"one": true, "two": true,
			},
		},
		{
			desc:  "Require max (fail)",
			exist: map[string]float64{"one": 0.5},
			goods: []*prodpb.Good{
				{
					Key: "one",
				},
				{
					Key: "two",
					Prereqs: []*prodpb.Requirement{
						{
							Key:    "one",
							Kind:   prodpb.Requirement_RK_MAX,
							Amount: 0.25,
						},
					},
				},
			},
			want: map[string]bool{
				"one": true,
			},
		},
		{
			desc:  "Require max (succeed)",
			exist: map[string]float64{"one": 0.5},
			goods: []*prodpb.Good{
				{
					Key: "one",
				},
				{
					Key: "two",
					Prereqs: []*prodpb.Requirement{
						{
							Key:    "one",
							Kind:   prodpb.Requirement_RK_MAX,
							Amount: 0.75,
						},
					},
				},
			},
			want: map[string]bool{
				"one": true, "two": true,
			},
		},
		{
			desc:  "Require cap (fail)",
			exist: map[string]float64{"one": 1.0, "two": 0.5},
			goods: []*prodpb.Good{
				{
					Key: "one",
				},
				{
					Key: "two",
					Prereqs: []*prodpb.Requirement{
						{
							Key:    "one",
							Kind:   prodpb.Requirement_RK_CAP,
							Amount: 0.5,
						},
					},
				},
			},
			want: map[string]bool{
				"one": true,
			},
		},
		{
			desc:  "Require cap (succeed)",
			exist: map[string]float64{"one": 0.6, "two": 0.5},
			goods: []*prodpb.Good{
				{
					Key: "one",
				},
				{
					Key: "two",
					Prereqs: []*prodpb.Requirement{
						{
							Key:    "one",
							Kind:   prodpb.Requirement_RK_CAP,
							Amount: 1.0,
						},
					},
				},
			},
			want: map[string]bool{
				"one": true, "two": true,
			},
		},
		{
			desc:  "Combos",
			exist: map[string]float64{"one": 0.5, "two": 0.25, "three": 0.33, "four": 1.0},
			goods: []*prodpb.Good{
				{
					Key:     "one",
					Maximum: 1.0,
					Prereqs: []*prodpb.Requirement{
						{
							Key:  "two",
							Kind: prodpb.Requirement_RK_FULL,
						},
						{
							Key:    "three",
							Kind:   prodpb.Requirement_RK_CAP,
							Amount: 1.0,
						},
					},
				},
				{
					Key:     "two",
					Maximum: 1.0,
					Prereqs: []*prodpb.Requirement{
						{
							Key:  "four",
							Kind: prodpb.Requirement_RK_FULL,
						},
						{
							Key:    "one",
							Kind:   prodpb.Requirement_RK_MIN,
							Amount: 0.25,
						},
						{
							Key:    "three",
							Kind:   prodpb.Requirement_RK_MAX,
							Amount: 0.5,
						},
					},
				},
				{
					Key:     "three",
					Maximum: 1.0,
					Prereqs: []*prodpb.Requirement{
						{
							Key:  "four",
							Kind: prodpb.Requirement_RK_FULL,
						},
						{
							Key:    "one",
							Kind:   prodpb.Requirement_RK_MIN,
							Amount: 0.25,
						},
						{
							Key:    "two",
							Kind:   prodpb.Requirement_RK_MAX,
							Amount: 0.15,
						},
					},
				},
				{
					Key:     "four",
					Maximum: 1.0,
					Prereqs: []*prodpb.Requirement{
						{
							Key:  "one",
							Kind: prodpb.Requirement_RK_FULL,
						},
						{
							Key:    "two",
							Kind:   prodpb.Requirement_RK_MIN,
							Amount: 0.25,
						},
						{
							Key:    "three",
							Kind:   prodpb.Requirement_RK_MAX,
							Amount: 0.50,
						},
					},
				},
			},
			want: map[string]bool{
				"two": true,
			},
		},
	}

	for _, cc := range cases {
		t.Run(cc.desc, func(t *testing.T) {
			buckets := make(map[string]*prodpb.Good)
			for _, g := range cc.goods {
				buckets[g.GetKey()] = g
			}
			got := filterGoods(cc.exist, buckets)
			for key := range got {
				if !cc.want[key] {
					t.Errorf("%s: filterGoods() => %q, not wanted.", cc.desc, key)
				}
				delete(cc.want, key)
			}
			if len(cc.want) > 0 {
				t.Errorf("%s: filterGoods() => %v, missing %v", cc.desc, got, cc.want)
			}
		})
	}
}
