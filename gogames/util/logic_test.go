package logic

import (
	"testing"

	lpb "gogames/util/logic_go_proto"
)

func TestBasics(t *testing.T) {
	defaults := NewTestLookup().
		WithInt("one", 1).
		WithInt("en", 1).
		WithInt("two", 2).
		WithStr("string1", "yohoho").
		WithStr("string2", "yohoho").
		WithStr("string3", "bwahaha").
		WithStrArr("strarr1", []string{"yohoho", "bwahaha"})

	cases := []struct {
		desc   string
		pred   *lpb.Predicate
		lookup Lookup
		want   bool
	}{
		{
			desc: "Greater than (false)",
			pred: &lpb.Predicate{
				Test: &lpb.Predicate_Comp{
					Comp: &lpb.Compare{
						KeyOne:    "one",
						KeyTwo:    "two",
						Operation: lpb.Compare_CMP_GT,
					},
				},
			},
			lookup: defaults,
			want:   false,
		},
		{
			desc: "Greater than (true)",
			pred: &lpb.Predicate{
				Test: &lpb.Predicate_Comp{
					Comp: &lpb.Compare{
						KeyOne:    "two",
						KeyTwo:    "one",
						Operation: lpb.Compare_CMP_GT,
					},
				},
			},
			lookup: defaults,
			want:   true,
		},
		{
			desc: "Less than (false)",
			pred: &lpb.Predicate{
				Test: &lpb.Predicate_Comp{
					Comp: &lpb.Compare{
						KeyOne:    "two",
						KeyTwo:    "one",
						Operation: lpb.Compare_CMP_LT,
					},
				},
			},
			lookup: defaults,
			want:   false,
		},
		{
			desc: "Less than (true)",
			pred: &lpb.Predicate{
				Test: &lpb.Predicate_Comp{
					Comp: &lpb.Compare{
						KeyOne:    "one",
						KeyTwo:    "two",
						Operation: lpb.Compare_CMP_LT,
					},
				},
			},
			lookup: defaults,
			want:   true,
		},
		{
			desc: "Equal (false)",
			pred: &lpb.Predicate{
				Test: &lpb.Predicate_Comp{
					Comp: &lpb.Compare{
						KeyOne:    "two",
						KeyTwo:    "one",
						Operation: lpb.Compare_CMP_EQ,
					},
				},
			},
			lookup: defaults,
			want:   false,
		},
		{
			desc: "Equal (true)",
			pred: &lpb.Predicate{
				Test: &lpb.Predicate_Comp{
					Comp: &lpb.Compare{
						KeyOne:    "one",
						KeyTwo:    "en",
						Operation: lpb.Compare_CMP_EQ,
					},
				},
			},
			lookup: defaults,
			want:   true,
		},
		{
			desc: "Greater than or equal (false)",
			pred: &lpb.Predicate{
				Test: &lpb.Predicate_Comp{
					Comp: &lpb.Compare{
						KeyOne:    "one",
						KeyTwo:    "two",
						Operation: lpb.Compare_CMP_GTE,
					},
				},
			},
			lookup: defaults,
			want:   false,
		},
		{
			desc: "Greater than or equal (true, greater)",
			pred: &lpb.Predicate{
				Test: &lpb.Predicate_Comp{
					Comp: &lpb.Compare{
						KeyOne:    "two",
						KeyTwo:    "one",
						Operation: lpb.Compare_CMP_GTE,
					},
				},
			},
			lookup: defaults,
			want:   true,
		},
		{
			desc: "Greater than or equal (true, equal)",
			pred: &lpb.Predicate{
				Test: &lpb.Predicate_Comp{
					Comp: &lpb.Compare{
						KeyOne:    "en",
						KeyTwo:    "one",
						Operation: lpb.Compare_CMP_GTE,
					},
				},
			},
			lookup: defaults,
			want:   true,
		},
		{
			desc: "Less than or equal (false)",
			pred: &lpb.Predicate{
				Test: &lpb.Predicate_Comp{
					Comp: &lpb.Compare{
						KeyOne:    "two",
						KeyTwo:    "one",
						Operation: lpb.Compare_CMP_LTE,
					},
				},
			},
			lookup: defaults,
			want:   false,
		},
		{
			desc: "Less than or equal (true, less than)",
			pred: &lpb.Predicate{
				Test: &lpb.Predicate_Comp{
					Comp: &lpb.Compare{
						KeyOne:    "one",
						KeyTwo:    "two",
						Operation: lpb.Compare_CMP_LTE,
					},
				},
			},
			lookup: defaults,
			want:   true,
		},
		{
			desc: "Less than or equal (true, equal)",
			pred: &lpb.Predicate{
				Test: &lpb.Predicate_Comp{
					Comp: &lpb.Compare{
						KeyOne:    "one",
						KeyTwo:    "en",
						Operation: lpb.Compare_CMP_LTE,
					},
				},
			},
			lookup: defaults,
			want:   true,
		},
		{
			desc: "Not equal (false)",
			pred: &lpb.Predicate{
				Test: &lpb.Predicate_Comp{
					Comp: &lpb.Compare{
						KeyOne:    "two",
						KeyTwo:    "one",
						Operation: lpb.Compare_CMP_NEQ,
					},
				},
			},
			lookup: defaults,
			want:   true,
		},
		{
			desc: "Not equal (true)",
			pred: &lpb.Predicate{
				Test: &lpb.Predicate_Comp{
					Comp: &lpb.Compare{
						KeyOne:    "one",
						KeyTwo:    "en",
						Operation: lpb.Compare_CMP_NEQ,
					},
				},
			},
			lookup: defaults,
			want:   false,
		},
		{
			desc: "String equals (false)",
			pred: &lpb.Predicate{
				Test: &lpb.Predicate_Comp{
					Comp: &lpb.Compare{
						KeyOne:    "string1",
						KeyTwo:    "string3",
						Operation: lpb.Compare_CMP_STREQ,
					},
				},
			},
			lookup: defaults,
			want:   false,
		},
		{
			desc: "String equals (true)",
			pred: &lpb.Predicate{
				Test: &lpb.Predicate_Comp{
					Comp: &lpb.Compare{
						KeyOne:    "string1",
						KeyTwo:    "string2",
						Operation: lpb.Compare_CMP_STREQ,
					},
				},
			},
			lookup: defaults,
			want:   true,
		},
		{
			desc: "Integer literals",
			pred: &lpb.Predicate{
				Test: &lpb.Predicate_Comp{
					Comp: &lpb.Compare{
						KeyOne:    "1",
						KeyTwo:    "2",
						Operation: lpb.Compare_CMP_GT,
					},
				},
			},
			lookup: defaults,
			want:   false,
		},
		{
			desc: "Integer literal mixed with variable",
			pred: &lpb.Predicate{
				Test: &lpb.Predicate_Comp{
					Comp: &lpb.Compare{
						KeyOne:    "two",
						KeyTwo:    "1",
						Operation: lpb.Compare_CMP_GT,
					},
				},
			},
			lookup: defaults,
			want:   true,
		},
		{
			desc: "String literals",
			pred: &lpb.Predicate{
				Test: &lpb.Predicate_Comp{
					Comp: &lpb.Compare{
						KeyOne:    "'literal",
						KeyTwo:    "'another literal",
						Operation: lpb.Compare_CMP_STREQ,
					},
				},
			},
			lookup: defaults,
			want:   false,
		},
		{
			desc: "String literal mixed with lookup",
			pred: &lpb.Predicate{
				Test: &lpb.Predicate_Comp{
					Comp: &lpb.Compare{
						KeyOne:    "'yohoho",
						KeyTwo:    "string1",
						Operation: lpb.Compare_CMP_STREQ,
					},
				},
			},
			lookup: defaults,
			want:   true,
		},
		{
			desc: "String in array (false)",
			pred: &lpb.Predicate{
				Test: &lpb.Predicate_Comp{
					Comp: &lpb.Compare{
						KeyOne:    "'banana",
						KeyTwo:    "strarr1",
						Operation: lpb.Compare_CMP_STRIN,
					},
				},
			},
			lookup: defaults,
			want:   false,
		},
		{
			desc: "String in array (true)",
			pred: &lpb.Predicate{
				Test: &lpb.Predicate_Comp{
					Comp: &lpb.Compare{
						KeyOne:    "string1",
						KeyTwo:    "strarr1",
						Operation: lpb.Compare_CMP_STRIN,
					},
				},
			},
			lookup: defaults,
			want:   true,
		},
		{
			desc: "String in array literal (false)",
			pred: &lpb.Predicate{
				Test: &lpb.Predicate_Comp{
					Comp: &lpb.Compare{
						KeyOne:    "'carrot",
						KeyTwo:    "['apple, 'banana, string1]",
						Operation: lpb.Compare_CMP_STRIN,
					},
				},
			},
			lookup: defaults,
			want:   false,
		},
		{
			desc: "String in array literal (true)",
			pred: &lpb.Predicate{
				Test: &lpb.Predicate_Comp{
					Comp: &lpb.Compare{
						KeyOne:    "'yohoho",
						KeyTwo:    "['apple, 'banana, string1]",
						Operation: lpb.Compare_CMP_STRIN,
					},
				},
			},
			lookup: defaults,
			want:   true,
		},
	}

	for _, cc := range cases {
		t.Run(cc.desc, func(t *testing.T) {
			got, err := Eval(cc.pred, cc.lookup)
			if err != nil {
				t.Errorf("%s: Eval() => %v, want nil", cc.desc, err)
			}
			if got != cc.want {
				t.Errorf("%s: Eval() => %v, want %v", cc.desc, got, cc.want)
			}
		})
	}
}

func TestCombinations(t *testing.T) {
	defaults := NewTestLookup().
		WithInt("one", 1).
		WithInt("en", 1).
		WithInt("two", 2)

	cases := []struct {
		desc   string
		pred   *lpb.Predicate
		lookup Lookup
		want   bool
	}{
		{
			desc: "All (true)",
			pred: &lpb.Predicate{
				Test: &lpb.Predicate_Comb{
					Comb: &lpb.Combine{
						Operands: []*lpb.Predicate{
							&lpb.Predicate{
								Test: &lpb.Predicate_Comp{
									Comp: &lpb.Compare{
										KeyOne:    "one",
										KeyTwo:    "two",
										Operation: lpb.Compare_CMP_LT,
									},
								},
							},
							&lpb.Predicate{
								Test: &lpb.Predicate_Comp{
									Comp: &lpb.Compare{
										KeyOne:    "two",
										KeyTwo:    "one",
										Operation: lpb.Compare_CMP_GT,
									},
								},
							},
							&lpb.Predicate{
								Test: &lpb.Predicate_Comp{
									Comp: &lpb.Compare{
										KeyOne:    "one",
										KeyTwo:    "en",
										Operation: lpb.Compare_CMP_EQ,
									},
								},
							},
						},
						Operation: lpb.Combine_IF_ALL,
					},
				},
			},
			lookup: defaults,
			want:   true,
		},
		{
			desc: "All (false)",
			pred: &lpb.Predicate{
				Test: &lpb.Predicate_Comb{
					Comb: &lpb.Combine{
						Operands: []*lpb.Predicate{
							&lpb.Predicate{
								Test: &lpb.Predicate_Comp{
									Comp: &lpb.Compare{
										KeyOne:    "one",
										KeyTwo:    "two",
										Operation: lpb.Compare_CMP_LT,
									},
								},
							},
							&lpb.Predicate{
								Test: &lpb.Predicate_Comp{
									Comp: &lpb.Compare{
										KeyOne:    "two",
										KeyTwo:    "one",
										Operation: lpb.Compare_CMP_GT,
									},
								},
							},
							&lpb.Predicate{
								Test: &lpb.Predicate_Comp{
									Comp: &lpb.Compare{
										KeyOne:    "one",
										KeyTwo:    "two",
										Operation: lpb.Compare_CMP_EQ,
									},
								},
							},
						},
						Operation: lpb.Combine_IF_ALL,
					},
				},
			},
			lookup: defaults,
			want:   false,
		},
		{
			desc: "Any (true)",
			pred: &lpb.Predicate{
				Test: &lpb.Predicate_Comb{
					Comb: &lpb.Combine{
						Operands: []*lpb.Predicate{
							&lpb.Predicate{
								Test: &lpb.Predicate_Comp{
									Comp: &lpb.Compare{
										KeyOne:    "one",
										KeyTwo:    "two",
										Operation: lpb.Compare_CMP_GT,
									},
								},
							},
							&lpb.Predicate{
								Test: &lpb.Predicate_Comp{
									Comp: &lpb.Compare{
										KeyOne:    "two",
										KeyTwo:    "one",
										Operation: lpb.Compare_CMP_LT,
									},
								},
							},
							&lpb.Predicate{
								Test: &lpb.Predicate_Comp{
									Comp: &lpb.Compare{
										KeyOne:    "one",
										KeyTwo:    "two",
										Operation: lpb.Compare_CMP_NEQ,
									},
								},
							},
						},
						Operation: lpb.Combine_IF_ANY,
					},
				},
			},
			lookup: defaults,
			want:   true,
		},
		{
			desc: "Any (false)",
			pred: &lpb.Predicate{
				Test: &lpb.Predicate_Comb{
					Comb: &lpb.Combine{
						Operands: []*lpb.Predicate{
							&lpb.Predicate{
								Test: &lpb.Predicate_Comp{
									Comp: &lpb.Compare{
										KeyOne:    "one",
										KeyTwo:    "two",
										Operation: lpb.Compare_CMP_GTE,
									},
								},
							},
							&lpb.Predicate{
								Test: &lpb.Predicate_Comp{
									Comp: &lpb.Compare{
										KeyOne:    "two",
										KeyTwo:    "one",
										Operation: lpb.Compare_CMP_LTE,
									},
								},
							},
							&lpb.Predicate{
								Test: &lpb.Predicate_Comp{
									Comp: &lpb.Compare{
										KeyOne:    "one",
										KeyTwo:    "two",
										Operation: lpb.Compare_CMP_EQ,
									},
								},
							},
						},
						Operation: lpb.Combine_IF_ANY,
					},
				},
			},
			lookup: defaults,
			want:   false,
		},
		{
			desc: "None (false)",
			pred: &lpb.Predicate{
				Test: &lpb.Predicate_Comb{
					Comb: &lpb.Combine{
						Operands: []*lpb.Predicate{
							&lpb.Predicate{
								Test: &lpb.Predicate_Comp{
									Comp: &lpb.Compare{
										KeyOne:    "one",
										KeyTwo:    "two",
										Operation: lpb.Compare_CMP_GT,
									},
								},
							},
							&lpb.Predicate{
								Test: &lpb.Predicate_Comp{
									Comp: &lpb.Compare{
										KeyOne:    "two",
										KeyTwo:    "one",
										Operation: lpb.Compare_CMP_LT,
									},
								},
							},
							&lpb.Predicate{
								Test: &lpb.Predicate_Comp{
									Comp: &lpb.Compare{
										KeyOne:    "one",
										KeyTwo:    "two",
										Operation: lpb.Compare_CMP_NEQ,
									},
								},
							},
						},
						Operation: lpb.Combine_IF_NONE,
					},
				},
			},
			lookup: defaults,
			want:   false,
		},
		{
			desc: "None (true)",
			pred: &lpb.Predicate{
				Test: &lpb.Predicate_Comb{
					Comb: &lpb.Combine{
						Operands: []*lpb.Predicate{
							&lpb.Predicate{
								Test: &lpb.Predicate_Comp{
									Comp: &lpb.Compare{
										KeyOne:    "one",
										KeyTwo:    "two",
										Operation: lpb.Compare_CMP_GTE,
									},
								},
							},
							&lpb.Predicate{
								Test: &lpb.Predicate_Comp{
									Comp: &lpb.Compare{
										KeyOne:    "two",
										KeyTwo:    "one",
										Operation: lpb.Compare_CMP_LTE,
									},
								},
							},
							&lpb.Predicate{
								Test: &lpb.Predicate_Comp{
									Comp: &lpb.Compare{
										KeyOne:    "one",
										KeyTwo:    "two",
										Operation: lpb.Compare_CMP_EQ,
									},
								},
							},
						},
						Operation: lpb.Combine_IF_NONE,
					},
				},
			},
			lookup: defaults,
			want:   true,
		},
	}

	for _, cc := range cases {
		t.Run(cc.desc, func(t *testing.T) {
			got, err := Eval(cc.pred, cc.lookup)
			if err != nil {
				t.Errorf("%s: Eval() => %v, want nil", cc.desc, err)
			}
			if got != cc.want {
				t.Errorf("%s: Eval() => %v, want %v", cc.desc, got, cc.want)
			}
		})
	}
}

func TestScopes(t *testing.T) {
	cases := []struct {
		desc   string
		base   *TestLookup
		scopes map[string]*TestLookup
		pred   *lpb.Predicate
		want   bool
	}{
		{
			desc: "Compare base and scope",
			base: NewTestLookup().WithStr("something", "abc"),
			scopes: map[string]*TestLookup{
				"scope": NewTestLookup().WithStr("another", "abc"),
			},
			pred: &lpb.Predicate{
				Test: &lpb.Predicate_Comp{
					Comp: &lpb.Compare{
						KeyOne:    "something",
						KeyTwo:    "scope.another",
						Operation: lpb.Compare_CMP_STREQ,
					},
				},
			},
			want: true,
		},
		{
			desc: "Base string in scope array",
			base: NewTestLookup().WithStr("something", "abc"),
			scopes: map[string]*TestLookup{
				"scope": NewTestLookup().WithStrArr("another", []string{"abc", "def"}),
			},
			pred: &lpb.Predicate{
				Test: &lpb.Predicate_Comp{
					Comp: &lpb.Compare{
						KeyOne:    "something",
						KeyTwo:    "scope.another",
						Operation: lpb.Compare_CMP_STRIN,
					},
				},
			},
			want: true,
		},
		{
			desc: "String literal in scope array",
			base: NewTestLookup(),
			scopes: map[string]*TestLookup{
				"scope": NewTestLookup().WithStrArr("another", []string{"abc", "def"}),
			},
			pred: &lpb.Predicate{
				Test: &lpb.Predicate_Comp{
					Comp: &lpb.Compare{
						KeyOne:    "'def",
						KeyTwo:    "scope.another",
						Operation: lpb.Compare_CMP_STRIN,
					},
				},
			},
			want: true,
		},
		{
			desc: "String from one scope in array from another",
			base: NewTestLookup(),
			scopes: map[string]*TestLookup{
				"tele":  NewTestLookup().WithStr("foo", "abc"),
				"scope": NewTestLookup().WithStrArr("another", []string{"abc", "def"}),
			},
			pred: &lpb.Predicate{
				Test: &lpb.Predicate_Comp{
					Comp: &lpb.Compare{
						KeyOne:    "tele.foo",
						KeyTwo:    "scope.another",
						Operation: lpb.Compare_CMP_STRIN,
					},
				},
			},
			want: true,
		},
		{
			desc: "String from scope in array literal",
			base: NewTestLookup(),
			scopes: map[string]*TestLookup{
				"tele": NewTestLookup().WithStr("foo", "abc"),
			},
			pred: &lpb.Predicate{
				Test: &lpb.Predicate_Comp{
					Comp: &lpb.Compare{
						KeyOne:    "tele.foo",
						KeyTwo:    "['abc, 'def]",
						Operation: lpb.Compare_CMP_STRIN,
					},
				},
			},
			want: true,
		},
	}

	for _, cc := range cases {
		t.Run(cc.desc, func(t *testing.T) {
			for key, scope := range cc.scopes {
				cc.base.SetScope(key, scope)
			}
			got, err := Eval(cc.pred, cc.base)
			if err != nil {
				t.Fatalf("%s: Eval() => %v, want nil", cc.desc, err)
			}
			if got != cc.want {
				t.Errorf("%s: Eval() => %v, want %v", cc.desc, got, cc.want)
			}
		})
	}
}
