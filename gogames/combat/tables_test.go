package tables

import (
	"strings"
	"testing"

	//"github.com/google/go-cmp/cmp"
	//"google.golang.org/protobuf/encoding/prototext"
	//"google.golang.org/protobuf/testing/protocmp"

	tpb "gogames/combat/tables_go_proto"
	//lpb "gogames/util/logic_go_proto"
)

func TestValidation(t *testing.T) {
	cases := []struct {
		desc   string
		lookup *Lookup
		phases []*tpb.Phase
		errors []string
	}{
		{
			desc:   "Nil",
			errors: []string{"nil tables.Lookup"},
		},
		{
			desc: "Trivial case",
			phases: []*tpb.Phase{
				&tpb.Phase{
					Key: "start_and_finish",
					Results: []*tpb.Result{
						&tpb.Result{Ends: true},
					},
				},
			},
			lookup: NewLookup(),
		},
		{
			desc: "Empty key",
			phases: []*tpb.Phase{
				&tpb.Phase{
					Results: []*tpb.Result{},
				},
			},
			lookup: NewLookup(),
			errors: []string{"empty key", "no ending state"},
		},
		{
			desc: "Duplicate key",
			phases: []*tpb.Phase{
				&tpb.Phase{
					Key: "start_and_finish",
					Results: []*tpb.Result{
						&tpb.Result{Ends: true},
					},
				},
				&tpb.Phase{
					Key: "start_and_finish",
					Results: []*tpb.Result{
						&tpb.Result{Ends: true},
					},
				},
			},
			lookup: NewLookup(),
			errors: []string{"duplicate key"},
		},
		{
			desc: "No results",
			phases: []*tpb.Phase{
				&tpb.Phase{
					Key: "start",
					Results: []*tpb.Result{
						&tpb.Result{Goto: "middle"},
					},
				},
				&tpb.Phase{
					Key: "middle",
				},
				&tpb.Phase{
					Key: "finish",
					Results: []*tpb.Result{
						&tpb.Result{Ends: true},
					},
				},
			},
			lookup: NewLookup(),
			errors: []string{"no results"},
		},
		{
			desc: "End and next",
			phases: []*tpb.Phase{
				&tpb.Phase{
					Key: "start",
					Results: []*tpb.Result{
						&tpb.Result{Goto: "middle", Ends: true},
					},
				},
				&tpb.Phase{
					Key: "middle",
					Results: []*tpb.Result{
						&tpb.Result{Goto: "finish"},
					},
				},
				&tpb.Phase{
					Key: "finish",
					Results: []*tpb.Result{
						&tpb.Result{Ends: true},
					},
				},
			},
			lookup: NewLookup(),
			errors: []string{"both ends and goes"},
		},
		{
			desc: "Goes to unknown phase",
			phases: []*tpb.Phase{
				&tpb.Phase{
					Key: "start",
					Results: []*tpb.Result{
						&tpb.Result{Goto: "whatever"},
					},
				},
				&tpb.Phase{
					Key: "middle",
					Results: []*tpb.Result{
						&tpb.Result{Goto: "finish"},
					},
				},
				&tpb.Phase{
					Key: "finish",
					Results: []*tpb.Result{
						&tpb.Result{Ends: true},
					},
				},
			},
			lookup: NewLookup(),
			errors: []string{"goes to nonexistent"},
		},
		{
			desc: "Happy case",
			phases: []*tpb.Phase{
				&tpb.Phase{
					Key: "start",
					Results: []*tpb.Result{
						&tpb.Result{Goto: "middle"},
					},
				},
				&tpb.Phase{
					Key: "middle",
					Results: []*tpb.Result{
						&tpb.Result{Goto: "finish"},
					},
				},
				&tpb.Phase{
					Key: "finish",
					Results: []*tpb.Result{
						&tpb.Result{Ends: true},
					},
				},
			},
			lookup: NewLookup(),
		},
	}

	for _, cc := range cases {
		t.Run(cc.desc, func(t *testing.T) {
			got := cc.lookup.AddPhases(cc.phases)
			if len(got) != len(cc.errors) {
				t.Fatalf("%s: AddPhases() => %v, want %v", cc.desc, got, cc.errors)
			}
			for idx, want := range cc.errors {
				if !strings.Contains(got[idx].Error(), want) {
					t.Errorf("%s: AddPhases(%d) => %v, want %q", cc.desc, idx, got[idx], want)
				}
			}
		})
	}
}
