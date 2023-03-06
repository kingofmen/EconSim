package mobile

import (
	"strings"
	"testing"

	"gogames/util/coords"
)

func TestSanity(t *testing.T) {
	cases := []struct {
		desc string
		op   func(m *Mobile) error
		err  string
		mob  *Mobile
		loc  coords.Point
	}{
		{
			desc: "Nil",
			op: func(m *Mobile) error {
				return m.Move(0, coords.New(0, 0))
			},
			err: "nil mobile",
		},
		{
			desc: "Too costly",
			op: func(m *Mobile) error {
				m.ResetMove(12)
				return m.Move(50, coords.New(0, 0))
			},
			err: "cannot move distance",
			mob: New(),
		},
		{
			desc: "Success",
			op: func(m *Mobile) error {
				m.ResetMove(100)
				return m.Move(50, coords.New(1, 1))
			},
			mob: New(),
			loc: coords.New(1, 1),
		},
	}

	for _, cc := range cases {
		t.Run(cc.desc, func(t *testing.T) {
			err := cc.op(cc.mob)
			if len(cc.err) > 0 {
				if err == nil || !strings.Contains(err.Error(), cc.err) {
					t.Errorf("%s: Got error %v, expect %q", cc.desc, err, cc.err)
				}
				return
			}
			if err != nil {
				t.Errorf("%s: Got error %v, want nil", cc.desc, err)
			}
			if cc.mob.Point != cc.loc {
				t.Errorf("%s: Final location %s, expected %s", cc.desc, cc.mob.Point, cc.loc)
			}
		})
	}
}
