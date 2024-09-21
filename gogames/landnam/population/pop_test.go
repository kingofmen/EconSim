package pop

import (
	"strings"
	"testing"

	"github.com/google/go-cmp/cmp"
	"google.golang.org/protobuf/encoding/prototext"
	"google.golang.org/protobuf/testing/protocmp"

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
			mgr := NewManager()
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

func TestProduction(t *testing.T) {
	cases := []struct {
		desc string
		tmps []*poppb.PopType
		pops []*poppb.Pop
		want []*poppb.Pop
		err  string
	}{
		{
			desc: "Happy case",
			tmps: []*poppb.PopType{
				&poppb.PopType{Key: "peasant", Production: 1000},
				&poppb.PopType{Key: "tenantry", Production: 2000},
			},
			pops: []*poppb.Pop{
				&poppb.Pop{Kind: "peasant"},
				&poppb.Pop{Kind: "tenantry"},
			},
			want: []*poppb.Pop{
				&poppb.Pop{Kind: "peasant", Prods: 1000},
				&poppb.Pop{Kind: "tenantry", Prods: 2000},
			},
		},
		{
			desc: "Bad key",
			pops: []*poppb.Pop{
				&poppb.Pop{Kind: "peasant", Prods: 1000},
				&poppb.Pop{Kind: "tenantry", Prods: 2000},
			},
			err: "bad type \"peasant\"",
		},
	}

	for _, cc := range cases {
		t.Run(cc.desc, func(t *testing.T) {
			mgr := NewManager()
			if errs := mgr.WithTypes(cc.tmps); len(errs) > 0 {
				t.Fatalf("%s: Error setting up templates: %v", cc.desc, errs)
			}

			err := mgr.Produce(cc.pops)
			if len(cc.err) > 0 {
				if err == nil {
					t.Errorf("%s: Produce() => nil, want %q", cc.desc, cc.err)
				} else if !strings.Contains(err.Error(), cc.err) {
					t.Errorf("%s: Produce() => %v, want %q", cc.desc, err, cc.err)
				}
				return
			}
			if len(cc.want) != len(cc.pops) {
				t.Fatalf("%s: Bad setup - %d input pops, %d outputs.", cc.desc, len(cc.pops), len(cc.want))
			}

			for idx, wp := range cc.want {
				if diff := cmp.Diff(wp, cc.pops[idx], protocmp.Transform()); len(diff) > 0 {
					t.Errorf("%s: Produce(%d) => %s, want %s, diff %s", cc.desc, idx, prototext.Format(cc.pops[idx]), prototext.Format(wp), diff)
				}
			}
		})
	}
}
