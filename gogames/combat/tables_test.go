package tables

import (
	"strings"
	"testing"

	"github.com/google/go-cmp/cmp"
	"gogames/util/logic"
	"google.golang.org/protobuf/encoding/prototext"
	"google.golang.org/protobuf/testing/protocmp"

	tpb "gogames/combat/tables_go_proto"
	lpb "gogames/util/logic_go_proto"
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
					Key:   "start_and_finish",
					Rolls: []int32{6},
					Results: []*tpb.Result{
						&tpb.Result{
							Ends: true,
							Pips: 1,
						},
					},
				},
			},
			lookup: NewLookup(),
		},
		{
			desc: "Missing pips",
			phases: []*tpb.Phase{
				&tpb.Phase{
					Key:   "start_and_finish",
					Rolls: []int32{6},
					Results: []*tpb.Result{
						&tpb.Result{
							Ends: true,
						},
					},
				},
			},
			lookup: NewLookup(),
			errors: []string{"fewer pips than 1"},
		},
		{
			desc: "Empty key",
			phases: []*tpb.Phase{
				&tpb.Phase{
					Rolls: []int32{6},
					Results: []*tpb.Result{
						&tpb.Result{
							Pips: 1,
						},
					},
				},
			},
			lookup: NewLookup(),
			errors: []string{"empty key", "no ending state"},
		},
		{
			desc: "Duplicate key",
			phases: []*tpb.Phase{
				&tpb.Phase{
					Key:   "start_and_finish",
					Rolls: []int32{6},
					Results: []*tpb.Result{
						&tpb.Result{
							Ends: true,
							Pips: 1,
						},
					},
				},
				&tpb.Phase{
					Key: "start_and_finish",
					Results: []*tpb.Result{
						&tpb.Result{
							Ends: true,
							Pips: 1,
						},
					},
				},
			},
			lookup: NewLookup(),
			errors: []string{"duplicate key"},
		},
		{
			desc: "No rolls",
			phases: []*tpb.Phase{
				&tpb.Phase{
					Key: "start_and_finish",
					Results: []*tpb.Result{
						&tpb.Result{
							Ends: true,
							Pips: 1,
						},
					},
				},
			},
			lookup: NewLookup(),
			errors: []string{"has no dierolls"},
		},
		{
			desc: "No results",
			phases: []*tpb.Phase{
				&tpb.Phase{
					Key:   "start",
					Rolls: []int32{6},
					Results: []*tpb.Result{
						&tpb.Result{
							Goto: "middle",
							Pips: 1,
						},
					},
				},
				&tpb.Phase{
					Key:   "middle",
					Rolls: []int32{6},
				},
				&tpb.Phase{
					Key:   "finish",
					Rolls: []int32{6},
					Results: []*tpb.Result{
						&tpb.Result{
							Ends: true,
							Pips: 1,
						},
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
					Key:   "start",
					Rolls: []int32{6},
					Results: []*tpb.Result{
						&tpb.Result{
							Goto: "middle",
							Ends: true,
							Pips: 1,
						},
					},
				},
				&tpb.Phase{
					Key:   "middle",
					Rolls: []int32{6},
					Results: []*tpb.Result{
						&tpb.Result{
							Goto: "finish",
							Pips: 1,
						},
					},
				},
				&tpb.Phase{
					Key:   "finish",
					Rolls: []int32{6},
					Results: []*tpb.Result{
						&tpb.Result{
							Ends: true,
							Pips: 1,
						},
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
					Key:   "start",
					Rolls: []int32{6},
					Results: []*tpb.Result{
						&tpb.Result{
							Goto: "whatever",
							Pips: 1,
						},
					},
				},
				&tpb.Phase{
					Key:   "middle",
					Rolls: []int32{6},
					Results: []*tpb.Result{
						&tpb.Result{
							Goto: "finish",
							Pips: 1,
						},
					},
				},
				&tpb.Phase{
					Key:   "finish",
					Rolls: []int32{6},
					Results: []*tpb.Result{
						&tpb.Result{
							Ends: true,
							Pips: 1,
						},
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
					Key:   "start",
					Rolls: []int32{6},
					Results: []*tpb.Result{
						&tpb.Result{
							Goto: "middle",
							Pips: 1,
						},
					},
				},
				&tpb.Phase{
					Key:   "middle",
					Rolls: []int32{6},
					Results: []*tpb.Result{
						&tpb.Result{
							Goto: "finish",
							Pips: 1,
						},
					},
				},
				&tpb.Phase{
					Key:   "finish",
					Rolls: []int32{6},
					Results: []*tpb.Result{
						&tpb.Result{
							Ends: true,
							Pips: 1,
						},
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

// testDice provides a predetermined series of rolls;
// it implements DieRoller.
type testDice struct {
	idx   int
	rolls []int32
}

func (td *testDice) Roll(face int32) int32 {
	td.idx++
	if nr := len(td.rolls) - 1; nr < td.idx {
		return td.rolls[nr]
	}
	return td.rolls[td.idx-1]
}

func TestResolve(t *testing.T) {
	oneGTtwo := &lpb.Predicate{
		Test: &lpb.Predicate_Comp{
			Comp: &lpb.Compare{
				KeyOne:    "1",
				KeyTwo:    "2",
				Operation: lpb.Compare_CMP_GT,
			},
		},
	}
	twoGTone := &lpb.Predicate{
		Test: &lpb.Predicate_Comp{
			Comp: &lpb.Compare{
				KeyOne:    "2",
				KeyTwo:    "1",
				Operation: lpb.Compare_CMP_GT,
			},
		},
	}

	cases := []struct {
		desc     string
		dice     *testDice
		phases   []*tpb.Phase
		llookup  *logic.TestLookup
		startKey string
		want     *tpb.Outcome
	}{
		{
			desc: "Exercise all features",
			dice: &testDice{
				rolls: []int32{1, 2, 1, 2, 1, 2},
			},
			phases: []*tpb.Phase{
				&tpb.Phase{
					Key:   "start",
					Rolls: []int32{6},
					Results: []*tpb.Result{
						&tpb.Result{
							Key:  "start_reroll",
							Pips: int32(2),
							Goto: "start",
						},
						&tpb.Result{
							Key:  "start_next",
							Pips: int32(1),
							Goto: "middle",
						},
					},
					Modifiers: []*tpb.Modifier{
						&tpb.Modifier{
							Key:      "start_plus_one",
							Value:    int32(1),
							Requires: []*lpb.Predicate{twoGTone},
						},
					},
				},
				&tpb.Phase{
					Key:   "middle",
					Rolls: []int32{6},
					Results: []*tpb.Result{
						&tpb.Result{
							Key:  "middle_reroll",
							Pips: int32(1),
							Goto: "middle",
						},
						&tpb.Result{
							Key:  "middle_next",
							Pips: int32(1),
							Goto: "finish",
						},
					},
					Modifiers: []*tpb.Modifier{
						&tpb.Modifier{
							Key:      "middle_minus_one",
							Value:    int32(-11),
							Requires: []*lpb.Predicate{oneGTtwo},
						},
					},
				},
				&tpb.Phase{
					Key:   "finish",
					Rolls: []int32{6},
					Results: []*tpb.Result{
						&tpb.Result{
							Key:  "finish_reroll",
							Pips: int32(1),
							Goto: "finish",
							Flags: []*tpb.Modifier{
								&tpb.Modifier{
									Key:      "set_this_flag",
									Value:    1,
									Requires: []*lpb.Predicate{twoGTone},
								},
								&tpb.Modifier{
									Key:      "not_this_flag",
									Value:    1,
									Requires: []*lpb.Predicate{oneGTtwo},
								},
							},
						},
						&tpb.Result{
							Key:  "end",
							Pips: int32(1),
							Ends: true,
						},
					},
				},
			},
			llookup:  logic.NewTestLookup(),
			startKey: "start",
			want: &tpb.Outcome{
				Flags: map[string]int32{
					"set_this_flag": 1,
				},
				Phases: []*tpb.Phase{
					&tpb.Phase{
						Key: "start",
						Results: []*tpb.Result{
							&tpb.Result{
								Key:  "start_reroll",
								Pips: 1,
								Flags: []*tpb.Modifier{
									&tpb.Modifier{
										Key:   "start_plus_one",
										Value: int32(1),
									},
								},
							},
						},
					},
					&tpb.Phase{
						Key: "start",
						Results: []*tpb.Result{
							&tpb.Result{
								Key:  "start_next",
								Pips: 2,
								Flags: []*tpb.Modifier{
									&tpb.Modifier{
										Key:   "start_plus_one",
										Value: int32(1),
									},
								},
							},
						},
					},
					&tpb.Phase{
						Key: "middle",
						Results: []*tpb.Result{
							&tpb.Result{
								Key:  "middle_reroll",
								Pips: 1,
							},
						},
					},
					&tpb.Phase{
						Key: "middle",
						Results: []*tpb.Result{
							&tpb.Result{
								Key:  "middle_next",
								Pips: 2,
							},
						},
					},
					&tpb.Phase{
						Key: "finish",
						Results: []*tpb.Result{
							&tpb.Result{
								Key:  "finish_reroll",
								Pips: 1,
							},
						},
					},
					&tpb.Phase{
						Key: "finish",
						Results: []*tpb.Result{
							&tpb.Result{
								Key:  "end",
								Pips: 2,
							},
						},
					},
				},
			},
		},
	}

	for _, cc := range cases {
		t.Run(cc.desc, func(t *testing.T) {
			lookup := NewLookup()
			if errs := lookup.AddPhases(cc.phases); len(errs) > 0 {
				t.Fatalf("%s: AddPhases() => %v, want nil", cc.desc, errs)
			}
			outcome, err := lookup.Resolve(cc.startKey, cc.dice, cc.llookup)
			if err != nil {
				t.Errorf("%s: Resolve() => %v, want nil", cc.desc, err)
			}
			if diff := cmp.Diff(outcome, cc.want, protocmp.Transform()); len(diff) > 0 {
				t.Errorf("%s: Resolve() => %s, want %s, diff %s", cc.desc, prototext.Format(outcome), prototext.Format(cc.want), diff)
			}
		})
	}
}

func TestResolveErrors(t *testing.T) {
	// TODO
}
