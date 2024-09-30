package pop

import (
	"strings"
	"testing"

	"github.com/google/go-cmp/cmp"
	"google.golang.org/protobuf/encoding/prototext"
	"google.golang.org/protobuf/testing/protocmp"

	poppb "gogames/landnam/population/pop_go_proto"
	lpb "gogames/util/logic_go_proto"
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
				&poppb.PopType{Key: "peasant", Produce: 1000},
				&poppb.PopType{Key: "tenantry", Produce: 2000},
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
			err: "unknown key \"peasant\"",
		},
	}

	for _, cc := range cases {
		t.Run(cc.desc, func(t *testing.T) {
			mgr := NewManager()
			if errs := mgr.WithTypes(cc.tmps); len(errs) > 0 {
				t.Fatalf("%s: Error setting up templates: %v", cc.desc, errs)
			}

			trades := make(map[*poppb.Pop][]*poppb.Pop)
			err := mgr.Produce(cc.pops, trades)
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

func TestConsumption(t *testing.T) {
	cases := []struct {
		desc string
		tmps []*poppb.PopType
		pops []*poppb.Pop
		want []*poppb.Pop
		err  string
	}{
		{
			desc: "Minimum consumption",
			tmps: []*poppb.PopType{
				&poppb.PopType{Key: "peasant", MinConsume: 1000, Capital: []int32{1000}},
			},
			pops: []*poppb.Pop{
				&poppb.Pop{Kind: "peasant", Prods: 1000},
			},
			want: []*poppb.Pop{
				&poppb.Pop{Kind: "peasant", Consume: 1000},
			},
		},
		{
			desc: "Priorities respected",
			tmps: []*poppb.PopType{
				&poppb.PopType{
					Key:        "peasant",
					MinConsume: 1000,
					Consume:    []int32{1000},
					Capital:    []int32{750},
					Militia:    []int32{500},
					Meaning:    []int32{250},
				},
			},
			pops: []*poppb.Pop{
				&poppb.Pop{Kind: "peasant", Prods: 7000},
			},
			want: []*poppb.Pop{
				&poppb.Pop{Kind: "peasant", Consume: 4000, Capital: 1500, Militia: 1000, Meaning: 500},
			},
		},
		{
			desc: "No priorities handled",
			tmps: []*poppb.PopType{
				&poppb.PopType{Key: "peasant"},
			},
			pops: []*poppb.Pop{
				&poppb.Pop{Kind: "peasant", Prods: 1000},
			},
			want: []*poppb.Pop{
				&poppb.Pop{Kind: "peasant", Prods: 1000},
			},
		},
		{
			desc: "Size increments consumption",
			tmps: []*poppb.PopType{
				&poppb.PopType{
					Key:        "peasant",
					Consume:    []int32{1000},
					Capital:    []int32{1000},
					SizConsume: 1000,
					IncConsume: 100,
				},
			},
			pops: []*poppb.Pop{
				&poppb.Pop{
					Kind:   "peasant",
					Prods:  1200,
					People: &poppb.Demographics{Adults: 1001},
				},
			},
			want: []*poppb.Pop{
				&poppb.Pop{
					Kind:    "peasant",
					Consume: 1100,
					Capital: 100,
					People:  &poppb.Demographics{Adults: 1001},
				},
			},
		},
		{
			desc: "Marginal use",
			tmps: []*poppb.PopType{
				&poppb.PopType{
					Key:     "peasant",
					Consume: []int32{1000, 1000, 0},
					Capital: []int32{1000, 500},
					Militia: []int32{100},
				},
			},
			pops: []*poppb.Pop{
				&poppb.Pop{
					Kind:   "peasant",
					Prods:  4900,
					People: &poppb.Demographics{Adults: 1001},
				},
			},
			want: []*poppb.Pop{
				&poppb.Pop{
					Kind:    "peasant",
					Consume: 2000,
					Capital: 2500,
					Militia: 400,
					People:  &poppb.Demographics{Adults: 1001},
				},
			},
		},
		{
			desc: "Error is no-op",
			tmps: []*poppb.PopType{
				&poppb.PopType{Key: "peasant", MinConsume: 1000, Consume: []int32{1000}},
			},
			pops: []*poppb.Pop{
				&poppb.Pop{Kind: "peasant", Prods: 2000},
				&poppb.Pop{Kind: "tenantry", Prods: 2000},
			},
			want: []*poppb.Pop{
				&poppb.Pop{Kind: "peasant", Prods: 2000},
				&poppb.Pop{Kind: "tenantry", Prods: 2000},
			},
			err: "unknown key \"tenan",
		},
	}

	for _, cc := range cases {
		t.Run(cc.desc, func(t *testing.T) {
			mgr := NewManager()
			if errs := mgr.WithTypes(cc.tmps); len(errs) > 0 {
				t.Fatalf("%s: Error setting up templates: %v", cc.desc, errs)
			}

			err := mgr.Consume(cc.pops)
			if len(cc.err) > 0 {
				if err == nil {
					t.Errorf("%s: Consume() => nil, want %q", cc.desc, cc.err)
				} else if !strings.Contains(err.Error(), cc.err) {
					t.Errorf("%s: Consume() => %v, want %q", cc.desc, err, cc.err)
				}
			}
			if len(cc.want) != len(cc.pops) {
				t.Fatalf("%s: Bad setup - %d input pops, %d outputs.", cc.desc, len(cc.pops), len(cc.want))
			}

			for idx, wp := range cc.want {
				if diff := cmp.Diff(wp, cc.pops[idx], protocmp.Transform()); len(diff) > 0 {
					t.Errorf("%s: Consume(%d) => %s, want %s, diff %s", cc.desc, idx, prototext.Format(cc.pops[idx]), prototext.Format(wp), diff)
				}
			}
		})
	}
}

func TestBirthsAndDeaths(t *testing.T) {
	cases := []struct {
		desc string
		tmps []*poppb.PopType
		pops []*poppb.Pop
		want []*poppb.Pop
	}{
		{
			desc: "Births",
			tmps: []*poppb.PopType{
				&poppb.PopType{
					Key: "peasant",
					BirthsPerThousand: &poppb.Demographics{
						Youths: 500,
						Adults: 1000,
					},
				},
			},
			pops: []*poppb.Pop{
				&poppb.Pop{
					Kind: "peasant",
					People: &poppb.Demographics{
						Youths: 2,
						Adults: 1,
					},
				},
			},
			want: []*poppb.Pop{
				&poppb.Pop{
					Kind: "peasant",
					People: &poppb.Demographics{
						Infants: 2,
						Youths:  2,
						Adults:  1,
					},
				},
			},
		},
		{
			desc: "Aging",
			tmps: []*poppb.PopType{
				&poppb.PopType{
					Key: "peasant",
				},
			},
			pops: []*poppb.Pop{
				&poppb.Pop{
					Kind: "peasant",
					People: &poppb.Demographics{
						Infants:  5,
						Children: 5,
						Youths:   5,
						Adults:   40,
					},
				},
			},
			want: []*poppb.Pop{
				&poppb.Pop{
					Kind: "peasant",
					People: &poppb.Demographics{
						Infants:  4,
						Children: 5,
						Youths:   5,
						Adults:   40,
						Elders:   1,
					},
				},
			},
		},
		{
			desc: "Deaths",
			tmps: []*poppb.PopType{
				&poppb.PopType{
					Key: "peasant",
					DeathsPerThousand: &poppb.Demographics{
						Infants:  1000,
						Children: 1000,
						Youths:   1000,
						Adults:   1000,
						Elders:   1000,
					},
				},
			},
			pops: []*poppb.Pop{
				&poppb.Pop{
					Kind: "peasant",
					People: &poppb.Demographics{
						Infants:  1,
						Children: 1,
						Youths:   1,
						Adults:   1,
						Elders:   1,
					},
				},
			},
			want: []*poppb.Pop{
				&poppb.Pop{
					Kind:   "peasant",
					People: &poppb.Demographics{},
				},
			},
		},
		{
			desc: "Steady state",
			tmps: []*poppb.PopType{
				&poppb.PopType{
					Key: "peasant",
					BirthsPerThousand: &poppb.Demographics{
						Adults: 26,
					},
					DeathsPerThousand: &poppb.Demographics{
						Infants:  3,
						Children: 2,
						Youths:   1,
						Adults:   1,
						Elders:   10,
					},
				},
			},
			pops: []*poppb.Pop{
				&poppb.Pop{
					Kind: "peasant",
					People: &poppb.Demographics{
						Infants:  1318,
						Children: 1307,
						Youths:   1301,
						Adults:   10000,
						Elders:   25000,
					},
				},
			},
			want: []*poppb.Pop{
				&poppb.Pop{
					Kind: "peasant",
					People: &poppb.Demographics{
						Infants:  1312,
						Children: 1307,
						Youths:   1301,
						Adults:   10000,
						Elders:   25000,
					},
				},
			},
		},
	}

	for _, cc := range cases {
		t.Run(cc.desc, func(t *testing.T) {
			mgr := NewManager()
			if errs := mgr.WithTypes(cc.tmps); len(errs) > 0 {
				t.Fatalf("%s: Error setting up templates: %v", cc.desc, errs)
			}

			if err := mgr.Demographics(cc.pops); err != nil {
				t.Fatalf("%s: Demographics() => %v, want nil", cc.desc, err)
			}
			if len(cc.want) != len(cc.pops) {
				t.Fatalf("%s: Bad setup - %d input pops, %d outputs.", cc.desc, len(cc.pops), len(cc.want))
			}

			for idx, wp := range cc.want {
				if diff := cmp.Diff(wp, cc.pops[idx], protocmp.Transform()); len(diff) > 0 {
					t.Errorf("%s: Demographics(%d) => %s, want %s, diff %s", cc.desc, idx, prototext.Format(cc.pops[idx]), prototext.Format(wp), diff)
				}
			}
		})
	}
}

func TestGainsFromTrade(t *testing.T) {
	cases := []struct {
		desc   string
		pop    *poppb.Pop
		others []*poppb.Pop
		want   int32
	}{
		{
			desc: "No trades, no specialization",
			pop:  &poppb.Pop{},
		},
		{
			desc: "No trades, bad result",
			pop: &poppb.Pop{
				Specialize: &poppb.Pop_Specialization{
					Good:  "wool",
					Level: 1,
				},
			},
			want: -1,
		},
		{
			desc: "One trade",
			pop: &poppb.Pop{
				Specialize: &poppb.Pop_Specialization{
					Good:  "wool",
					Level: 1,
				},
			},
			others: []*poppb.Pop{
				&poppb.Pop{
					Specialize: &poppb.Pop_Specialization{
						Good:  "grain",
						Level: 1,
					},
				},
			},
			want: 1,
		},
		{
			desc: "Scales with level",
			pop: &poppb.Pop{
				Specialize: &poppb.Pop_Specialization{
					Good:  "wool",
					Level: 7,
				},
			},
			others: []*poppb.Pop{
				&poppb.Pop{
					Specialize: &poppb.Pop_Specialization{
						Good:  "grain",
						Level: 7,
					},
				},
			},
			want: 7,
		},
		{
			desc: "Two trades",
			pop: &poppb.Pop{
				Specialize: &poppb.Pop_Specialization{
					Good:  "wool",
					Level: 1,
				},
			},
			others: []*poppb.Pop{
				&poppb.Pop{
					Specialize: &poppb.Pop_Specialization{
						Good:  "grain",
						Level: 1,
					},
				},
				&poppb.Pop{
					Specialize: &poppb.Pop_Specialization{
						Good:  "salt",
						Level: 1,
					},
				},
			},
			want: 3,
		},
		{
			desc: "Duplicates don't matter",
			pop: &poppb.Pop{
				Specialize: &poppb.Pop_Specialization{
					Good:  "wool",
					Level: 1,
				},
			},
			others: []*poppb.Pop{
				&poppb.Pop{
					Specialize: &poppb.Pop_Specialization{
						Good:  "grain",
						Level: 1,
					},
				},
				&poppb.Pop{
					Specialize: &poppb.Pop_Specialization{
						Good:  "grain",
						Level: 1,
					},
				},
				&poppb.Pop{
					Specialize: &poppb.Pop_Specialization{
						Good:  "salt",
						Level: 1,
					},
				},
			},
			want: 3,
		},
		{
			desc: "Unspecialized is a small gain",
			pop: &poppb.Pop{
				Specialize: &poppb.Pop_Specialization{
					Good:  "wool",
					Level: 2,
				},
			},
			others: []*poppb.Pop{
				&poppb.Pop{},
				&poppb.Pop{},
				&poppb.Pop{
					Specialize: &poppb.Pop_Specialization{
						Good:  "grain",
						Level: 1,
					},
				},
				&poppb.Pop{
					Specialize: &poppb.Pop_Specialization{
						Good:  "grain",
						Level: 1,
					},
				},
				&poppb.Pop{
					Specialize: &poppb.Pop_Specialization{
						Good:  "salt",
						Level: 2,
					},
				},
				&poppb.Pop{
					Specialize: &poppb.Pop_Specialization{
						Good:  "salt",
						Level: 2,
					},
				},
			},
			want: 8,
		},
		{
			desc: "Unspecialized does not gain from trade",
			pop:  &poppb.Pop{},
			others: []*poppb.Pop{
				&poppb.Pop{},
				&poppb.Pop{
					Specialize: &poppb.Pop_Specialization{
						Good:  "grain",
						Level: 1,
					},
				},
				&poppb.Pop{
					Specialize: &poppb.Pop_Specialization{
						Good:  "grain",
						Level: 1,
					},
				},
				&poppb.Pop{
					Specialize: &poppb.Pop_Specialization{
						Good:  "salt",
						Level: 1,
					},
				},
			},
			want: 0,
		},
	}

	for _, cc := range cases {
		t.Run(cc.desc, func(t *testing.T) {
			if got := gainsFromTrade(1, cc.pop, cc.others); got != cc.want {
				t.Errorf("%s: gainsFromTrade() => %v, want %v", cc.desc, got, cc.want)
			}
		})
	}
}

func TestChangeMatch(t *testing.T) {
	manager := NewManager()
	if errs := manager.WithTypes([]*poppb.PopType{
		&poppb.PopType{Key: "peasant"},
		&poppb.PopType{Key: "merchant"},
		&poppb.PopType{Key: "knight"},
	}); len(errs) > 0 {
		t.Fatalf("error setting up POP types: %v", errs)
	}
	cases := []struct {
		desc   string
		pop    *poppb.Pop
		evolve *poppb.PopChange
		want   bool
	}{
		{
			desc: "Unallowed type",
			pop:  &poppb.Pop{Kind: "peasant"},
			evolve: &poppb.PopChange{
				Requires: []*lpb.Predicate{
					&lpb.Predicate{
						Test: &lpb.Predicate_Comb{
							Comb: &lpb.Combine{
								Operation: lpb.Combine_IF_ANY,
								Operands: []*lpb.Predicate{
									&lpb.Predicate{
										Test: &lpb.Predicate_Comp{
											Comp: &lpb.Compare{
												KeyOne:    "pop_kind",
												KeyTwo:    "knight",
												Operation: lpb.Compare_CMP_STREQ,
											},
										},
									},
									&lpb.Predicate{
										Test: &lpb.Predicate_Comp{
											Comp: &lpb.Compare{
												KeyOne:    "pop_kind",
												KeyTwo:    "merchant",
												Operation: lpb.Compare_CMP_STREQ,
											},
										},
									},
								},
							},
						},
					},
				},
			},
			want: false,
		},
		{
			desc: "Allowed type",
			pop:  &poppb.Pop{Kind: "peasant"},
			evolve: &poppb.PopChange{
				Requires: []*lpb.Predicate{
					&lpb.Predicate{
						Test: &lpb.Predicate_Comb{
							Comb: &lpb.Combine{
								Operation: lpb.Combine_IF_ANY,
								Operands: []*lpb.Predicate{
									&lpb.Predicate{
										Test: &lpb.Predicate_Comp{
											Comp: &lpb.Compare{
												KeyOne:    "pop_kind",
												KeyTwo:    "peasant",
												Operation: lpb.Compare_CMP_STREQ,
											},
										},
									},
									&lpb.Predicate{
										Test: &lpb.Predicate_Comp{
											Comp: &lpb.Compare{
												KeyOne:    "pop_kind",
												KeyTwo:    "merchant",
												Operation: lpb.Compare_CMP_STREQ,
											},
										},
									},
								},
							},
						},
					},
				},
			},
			want: true,
		},
	}

	for _, cc := range cases {
		t.Run(cc.desc, func(t *testing.T) {
			got, err := manager.match(cc.pop, cc.evolve)
			if err != nil {
				t.Errorf("%s: match() => %v, want nil", cc.desc, err)
			}
			if got != cc.want {
				t.Errorf("%s: match() => %v, want %v", cc.desc, got, cc.want)
			}
		})
	}
}

func TestApplyChange(t *testing.T) {
	cases := []struct {
		desc   string
		pop    *poppb.Pop
		evolve *poppb.PopChange
		want   *poppb.Pop
	}{
		{
			desc: "Base case",
			pop: &poppb.Pop{
				Kind: "peasant",
			},
			evolve: &poppb.PopChange{
				NewKey: "merchant",
			},
			want: &poppb.Pop{
				Kind: "merchant",
			},
		},
	}

	for _, cc := range cases {
		apply(cc.pop, cc.evolve)
		if diff := cmp.Diff(cc.pop, cc.want, protocmp.Transform()); len(diff) > 0 {
			t.Errorf("%s: apply() => %s, want %s, diff %s", cc.desc, prototext.Format(cc.pop), prototext.Format(cc.want), diff)
		}
	}
}
