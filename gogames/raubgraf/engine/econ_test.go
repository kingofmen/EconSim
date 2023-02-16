package econ

import (
  "strings"
  "testing"
)

func TestContract(t *testing.T) {
  cases := []struct{
    desc string
    start map[Resource]int
    transfer map[Resource]int
    err string
  } {
      {
        desc: "Insufficient food",
        start: map[Resource]int{},
        transfer: map[Resource]int{Food: 1},
        err: "not enough Food, have 0",
      },
      {
        desc: "Insufficient wealth",
        start: map[Resource]int{},
        transfer: map[Resource]int{Wealth: 1},
        err: "not enough Wealth, have 0",
      },
      {
        desc: "Works",
        start: map[Resource]int{Wealth: 1, Food: 1},
        transfer: map[Resource]int{Wealth: 1, Food: 1},
      },
    }

  for _, cc := range cases {
    t.Run(cc.desc, func(t *testing.T) {
      source := NewStore()
      target := NewStore()
      amount := NewStore()
      contract := NewContract().WithSource(source).WithAmount(amount).WithTarget(target)
      for r, v := range cc.start {
        source.AddResource(r, v)
      }
      for r, v := range cc.transfer {
        amount.AddResource(r, v)
      }
      err := contract.Execute()
      if len(cc.err) != 0 {
        if err == nil {
          t.Errorf("%s: Execute() => nil, want %q", cc.desc, cc.err)
        } else if !strings.Contains(err.Error(), cc.err) {
          t.Errorf("%s: Execute() => %v, want %q", cc.desc, err, cc.err)
        }
        return
      } else if err != nil {
          t.Errorf("%s: Execute() => %v, want nil", cc.desc, err)
      }

      for _, res := range []Resource{Food, Wealth} {
        if exp, got := cc.transfer[res], target.Count(res); exp != got {
          t.Errorf("%s: Execute() => target has %d %s, expect %d", cc.desc, got, res, exp)
        }
        if exp, got := cc.start[res] - cc.transfer[res], source.Count(res); exp != got {
          t.Errorf("%s: Execute() => source has %d %s, expect %d", cc.desc, got, res, exp)
        }
      }
    })
  }
}

func TestStore(t *testing.T) {
  deltas := []struct{
    add int
    exp int
    err string
  } {
      {
        add: 0,
        exp: 0,
      },
      {
        add: 1,
        exp: 1,
      },
      {
        add: 10,
        exp: 11,
      },
      {
        add: -1,
        exp: 10,
      },
      {
        add: -100,
        exp: 0,
        err: "subtract 100",
      },
    }

  ss := NewStore()
  for _, d := range deltas {
    err := ss.AddFood(d.add)
    if d.err == "" {
      if err != nil {
        t.Errorf("AddFood(%d) => %v, want nil", d.add, err)
      }
    } else {
      if err == nil || !strings.Contains(err.Error(), d.err) {
        t.Errorf("AddFood(%d) => %v, want %q", d.add, err, d.err)
      }
    }
    if got := ss.CountFood(); got != d.exp {
      t.Errorf("CountFood(%d) => %d, want %d", d.add, got, d.exp)
    }
    if got := ss.Count(Food); got != d.exp {
      t.Errorf("Count(Food (%d)) => %d, want %d", d.add, got, d.exp)
    }
    err = ss.AddWealth(d.add)
    if d.err == "" {
      if err != nil {
        t.Errorf("AddWealth(%d) => %v, want nil", d.add, err)
      }
    } else {
      if err == nil || !strings.Contains(err.Error(), d.err) {
        t.Errorf("AddWealth(%d) => %v, want %q", d.add, err, d.err)
      }
    }
    if got := ss.CountWealth(); got != d.exp {
      t.Errorf("CountWealth(%d) => %d, want %d", d.add, got, d.exp)
    }
    if got := ss.Count(Wealth); got != d.exp {
      t.Errorf("Count(Wealth (%d)) => %d, want %d", d.add, got, d.exp)
    }
  }
  ss = NewStore()
  for _, d := range deltas {
    err := ss.AddResource(Food, d.add)
    if d.err == "" {
      if err != nil {
        t.Errorf("AddResource(Food, %d) => %v, want nil", d.add, err)
      }
    } else {
      if err == nil || !strings.Contains(err.Error(), d.err) {
        t.Errorf("AddResource(Food, %d) => %v, want %q", d.add, err, d.err)
      }
    }
    if got := ss.CountFood(); got != d.exp {
      t.Errorf("CountFood(%d) => %d, want %d", d.add, got, d.exp)
    }
    err = ss.AddResource(Wealth, d.add)
    if d.err == "" {
      if err != nil {
        t.Errorf("AddResource(Wealth, %d) => %v, want nil", d.add, err)
      }
    } else {
      if err == nil || !strings.Contains(err.Error(), d.err) {
        t.Errorf("AddResource(Wealth, %d) => %v, want %q", d.add, err, d.err)
      }
    }
    if got := ss.CountWealth(); got != d.exp {
      t.Errorf("CountWealth(%d) => %d, want %d", d.add, got, d.exp)
    }
  }
}
