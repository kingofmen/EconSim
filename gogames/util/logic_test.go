package logic

import (
	"fmt"
	"testing"

	lpb "gogames/util/logic_go_proto"
)

type TestLookup struct {
	ints map[string]int32
	strs map[string]string
}

func testLookup() *TestLookup {
	return &TestLookup{
		ints: make(map[string]int32),
		strs: make(map[string]string),
	}
}

func (tl *TestLookup) GetInt(key string) (int32, error) {
	if tl == nil {
		return 0, fmt.Errorf("nil lookup object")
	}
	val, ok := tl.ints[key]
	if !ok {
		return 0, fmt.Errorf("unknown key %q", key)
	}
	return val, nil
}

func (tl *TestLookup) GetStr(key string) (string, error) {
	if tl == nil {
		return "", fmt.Errorf("nil lookup object")
	}
	val, ok := tl.strs[key]
	if !ok {
		return "", fmt.Errorf("unknown key %q", key)
	}
	return val, nil
}

func (tl *TestLookup) WithInt(key string, val int32) *TestLookup {
	if tl == nil {
		tl = testLookup()
	}
	tl.ints[key] = val
	return tl
}

func (tl *TestLookup) WithStr(key string, val string) *TestLookup {
	if tl == nil {
		tl = testLookup()
	}
	tl.strs[key] = val
	return tl
}

func TestBasics(t *testing.T) {
	defaults := testLookup().
		WithInt("one", 1).
		WithInt("en", 1).
		WithInt("two", 2).
		WithStr("string1", "yohoho").
		WithStr("string2", "yohoho").
		WithStr("string3", "bwahaha")

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
	defaults := testLookup().
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
