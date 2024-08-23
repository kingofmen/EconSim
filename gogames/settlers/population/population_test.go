package pop

import (
	"strings"
	"testing"

	poppb "gogames/settlers/population/population_proto"
)

func TestValidate(t *testing.T) {
	cases := []struct {
		desc string
		tmps []*poppb.Household
		want []string
	}{
		{
			desc: "Empty proto",
			tmps: []*poppb.Household{
				&poppb.Household{},
			},
			want: []string{
				"has empty lookup key",
				"bad size 0",
				"time 0, must be at least 1",
			},
		},
		{
			desc: "Valid proto",
			tmps: []*poppb.Household{
				&poppb.Household{
					Key:        "test",
					Size:       10,
					Military:   1,
					DoubleTime: 100,
				},
			},
			want: []string{},
		},
		{
			desc: "Duplicate",
			tmps: []*poppb.Household{
				&poppb.Household{
					Key:        "test",
					Size:       10,
					Military:   1,
					DoubleTime: 100,
				},
				&poppb.Household{
					Key:        "test",
					Size:       10,
					Military:   1,
					DoubleTime: 100,
				},
			},
			want: []string{"Duplicate lookup key"},
		},
		{
			desc: "Military too large",
			tmps: []*poppb.Household{
				&poppb.Household{
					Key:        "test",
					Size:       10,
					Military:   11,
					DoubleTime: 100,
				},
			},
			want: []string{"more military"},
		},
	}

	for _, cc := range cases {
		t.Run(cc.desc, func(t *testing.T) {
			got := Validate(cc.tmps)
			if len(got) != len(cc.want) {
				t.Errorf("%s: Received %d errors %v, want %d: %v", cc.desc, len(got), got, len(cc.want), cc.want)
				return
			}
			for i, err := range got {
				if !strings.Contains(err.Error(), cc.want[i]) {
					t.Errorf("%s: Error %d is %v, want %q", cc.desc, i, err, cc.want[i])
				}
			}
		})
	}
}
