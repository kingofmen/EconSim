package validate

import (
	"strings"
	"testing"

	cpb "gogames/settlers/economy/chain_proto"
)

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
			err := Web(cc.web)
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
