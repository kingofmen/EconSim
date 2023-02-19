package mobile

import(
  "strings"
  "testing"

  "gogames/util/coords"
)

func TestSanity(t *testing.T) {
  cases := []struct{
    desc string
    op func(m *Mobile) error
    err string
    mob *Mobile
  } {
      {
        desc: "Nil",
        op: func(m *Mobile) error {
          return m.Move(0, coords.New(0, 0))
        },
        err: "nil mobile",
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
    })
  }
}
