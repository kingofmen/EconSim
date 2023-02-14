package flags

import(
  "testing"
)

func TestCRUD(t *testing.T) {
  cases := []struct {
    desc string
    op func(fh *FlagHolder)
    exp map[string]int
  } {
      {
        desc: "Create one flag",
        op: func(fh *FlagHolder) {
          fh.AddFlag("test", 2)
        },
        exp: map[string]int{"test": 2},
      },
      {
        desc: "Create and increment",
        op: func(fh *FlagHolder) {
          fh.AddFlag("test", 2)
          fh.IncFlag("test")
        },
        exp: map[string]int{"test": 3},
      },
      {
        desc: "Create, update, clear",
        op: func(fh *FlagHolder) {
          fh.AddFlag("test", 2)
          fh.IncFlag("test")
          fh.ClearFlag("test")
        },
        exp: map[string]int{"test": 0},
      },
      {
        desc: "Multiple flags",
        op: func(fh *FlagHolder) {
          fh.AddFlag("foo", 2)
          fh.IncFlag("bar")
          fh.IncFlag("foo")
          fh.AddFlag("baz", 10)
          fh.ClearFlag("bar")
        },
        exp: map[string]int{"foo": 3, "bar": 0, "baz": 10},
      },
      {
        desc: "Multiple flags, clear all",
        op: func(fh *FlagHolder) {
          fh.AddFlag("foo", 2)
          fh.IncFlag("bar")
          fh.IncFlag("foo")
          fh.AddFlag("baz", 10)
          fh.Clear()
          fh.AddFlag("zoinks", 100)
        },
        exp: map[string]int{"foo": 0, "bar": 0, "baz": 0, "zoinks": 100},
      },
    }

  for _, cc := range cases {
    t.Run(cc.desc, func(t *testing.T) {
      fh := New()
      cc.op(fh)
      for k, v := range cc.exp {
        if got := fh.Value(k); got != v {
          t.Errorf("%s: Value(%s) => %d, want %d", cc.desc, k, got, v)
        }
        delete(fh.flags, k)
      }
      if len(fh.flags) > 0 {
        t.Errorf("%s: Unexpected flag values %v", cc.desc, fh.flags)
      }
    })
  }

}
