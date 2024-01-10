package settlers

import (
	"fmt"
	"testing"

	"gogames/util/coords"
)

type testRule struct {
	allow func(pos coords.Point, fac *Faction, board *Board) error
}

func (tr *testRule) Allow(pos coords.Point, fac *Faction, board *Board) error {
	return tr.allow(pos, fac, board)
}

func TestPlace(t *testing.T) {
	cases := []struct {
		desc string
		tmpl *Template
		want []error
	}{
		{
			desc: "Always rule",
			tmpl: &Template{
				required: []Rule{
					&testRule{
						allow: func(pos coords.Point, fac *Faction, board *Board) error {
							return nil
						},
					},
				},
			},
		},
		{
			desc: "Never rule",
			tmpl: &Template{
				required: []Rule{
					&testRule{
						allow: func(pos coords.Point, fac *Faction, board *Board) error {
							return fmt.Errorf("Never!")
						},
					},
				},
			},
			want: []error{fmt.Errorf("Never!")},
		},
		{
			desc: "Multiple rules",
			tmpl: &Template{
				required: []Rule{
					&testRule{
						allow: func(pos coords.Point, fac *Faction, board *Board) error {
							return fmt.Errorf("Never!")
						},
					},
					&testRule{
						allow: func(pos coords.Point, fac *Faction, board *Board) error {
							return nil
						},
					},
					&testRule{
						allow: func(pos coords.Point, fac *Faction, board *Board) error {
							return fmt.Errorf("Some other thing!")
						},
					},
				},
			},
			want: []error{fmt.Errorf("Never!"), fmt.Errorf("Some other thing!")},
		},
	}

	board := NewBoard()
	for _, cc := range cases {
		t.Run(cc.desc, func(t *testing.T) {
			errs := board.Place(coords.Nil(), nil, cc.tmpl)
			if len(errs) != len(cc.want) {
				t.Errorf("%s: Place() => %v, want %v", cc.desc, errs, cc.want)
				return
			}
			for idx, err := range errs {
				if err.Error() != cc.want[idx].Error() {
					t.Errorf("%s: Place() => error %d %v, want %v", cc.desc, idx, err, cc.want[idx])
				}
			}
		})
	}
}
