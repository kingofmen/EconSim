package pop

import (
	"strings"
	"testing"

	poppb "gogames/landnam/population/pop_go_proto"
)

func TestValidation(t *testing.T) {
	cases := []struct {
		desc string
		tmps []*poppb.PopType
		pops []*poppb.Pop
		want []string
	}{
		{
			desc: "Zero-length key",
			tmps: []*poppb.PopType{
				&poppb.PopType{},
			},
			want: []string{"zero-length key"},
		},
		{
			desc: "Duplicate key",
			tmps: []*poppb.PopType{
				&poppb.PopType{Key: "one"},
				&poppb.PopType{Key: "one"},
			},
			want: []string{"Duplicate key"},
		},
		{
			desc: "Bad keys",
			tmps: []*poppb.PopType{
				&poppb.PopType{Key: "one"},
				&poppb.PopType{Key: "two"},
			},
			pops: []*poppb.Pop{
				&poppb.Pop{Kind: ""},
				&poppb.Pop{Kind: "three"},
			},
			want: []string{"unknown key \"\"", "unknown key \"three\""},
		},
		{
			desc: "Happy case",
			tmps: []*poppb.PopType{
				&poppb.PopType{Key: "one"},
				&poppb.PopType{Key: "two"},
			},
			pops: []*poppb.Pop{
				&poppb.Pop{Kind: "one"},
				&poppb.Pop{Kind: "one"},
				&poppb.Pop{Kind: "two"},
			},
		},
	}

	for _, cc := range cases {
		t.Run(cc.desc, func(t *testing.T) {
			mgr := &Manager{}
			got := mgr.WithTypes(cc.tmps)
			got = append(got, mgr.Validate(cc.pops)...)
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
