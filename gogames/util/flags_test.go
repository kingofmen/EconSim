package flags

import(
  "testing"
)

func TestCRUD(t *testing.T) {
  cases := []struct {
    desc string
    op func(fh *Holder)
    exp map[string]int
  } {
      {
        desc: "Create one flag",
        op: func(fh *Holder) {
          fh.AddFlag("test", 2)
        },
        exp: map[string]int{"test": 2},
      },
      {
        desc: "Create and increment",
        op: func(fh *Holder) {
          fh.AddFlag("test", 2)
          fh.IncFlag("test")
        },
        exp: map[string]int{"test": 3},
      },
      {
        desc: "Create, update, clear",
        op: func(fh *Holder) {
          fh.AddFlag("test", 2)
          fh.IncFlag("test")
          fh.ClearFlag("test")
        },
        exp: map[string]int{"test": 0},
      },
      {
        desc: "Multiple flags",
        op: func(fh *Holder) {
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
        op: func(fh *Holder) {
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
        delete(fh.values, k)
      }
      if len(fh.values) > 0 {
        t.Errorf("%s: Unexpected flag values %v", cc.desc, fh.values)
      }
    })
  }
}

func TestLimits(t *testing.T) {
  cases := []struct {
    desc string
    op func(fh *Holder)
    exp map[string]int
    ceil map[string]int
    flr map[string]int
  } {
      {
        desc: "Ceiling then value",
        op: func(fh *Holder) {
          fh.SetCeiling("test", 3)
          fh.AddFlag("test", 4)
        },
        exp: map[string]int{"test": 3},
        ceil: map[string]int{"test": 3},
      },
      {
        desc: "Floor then value",
        op: func(fh *Holder) {
          fh.SetFloor("test", -3)
          fh.AddFlag("test", -4)
        },
        exp: map[string]int{"test": -3},
        flr: map[string]int{"test": -3},
      },
      {
        desc: "Ceiling caps value",
        op: func(fh *Holder) {
          fh.AddFlag("test", 4)
          fh.SetCeiling("test", 3)
        },
        exp: map[string]int{"test": 3},
        ceil: map[string]int{"test": 3},
      },
      {
        desc: "Floor caps value",
        op: func(fh *Holder) {
          fh.AddFlag("test", 1)
          fh.SetFloor("test", 2)
        },
        exp: map[string]int{"test": 2},
        flr: map[string]int{"test": 2},
      },
      {
        desc: "Floor limits ceiling",
        op: func(fh *Holder) {
          fh.SetFloor("test", -1)
          fh.SetCeiling("test", -2)
        },
        exp: map[string]int{"test": -1},
        ceil: map[string]int{"test": -1},
        flr: map[string]int{"test": -1},
      },
      {
        desc: "Ceiling caps floor",
        op: func(fh *Holder) {
          fh.SetCeiling("test", 2)
          fh.SetFloor("test", 3)
        },
        exp: map[string]int{"test": 2},
        ceil: map[string]int{"test": 2},
        flr: map[string]int{"test": 2},
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
        delete(fh.values, k)
      }
      if len(fh.values) > 0 {
        t.Errorf("%s: Unexpected flag values %v", cc.desc, fh.values)
      }
      for k, v := range cc.ceil {
        if got := fh.roofs[k]; got != v {
          t.Errorf("%s: Ceiling(%s) => %d, want %d", cc.desc, k, got, v)
        }
        delete(fh.roofs, k)
      }
      if len(fh.roofs) > 0 {
        t.Errorf("%s: Unexpected flag roofs %v", cc.desc, fh.roofs)
      }
      for k, v := range cc.flr {
        if got := fh.floors[k]; got != v {
          t.Errorf("%s: floor(%s) => %d, want %d", cc.desc, k, got, v)
        }
        delete(fh.floors, k)
      }
      if len(fh.floors) > 0 {
        t.Errorf("%s: Unexpected flag floors %v", cc.desc, fh.floors)
      }
    })
  }
}
