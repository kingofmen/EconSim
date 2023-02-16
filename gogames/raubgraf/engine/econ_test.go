package econ

import (
  "fmt"
  "strings"
  "testing"
)

type testFoodStore struct{
  food int
}

func (fs *testFoodStore) CountFood() int {
  if fs == nil {
    return 0
  }
  return fs.food
}

func (fs *testFoodStore) AddFood(a int) error {
  if fs == nil {
    return nil
  }
  fs.food += a
  if fs.food < 0 {
    fs.food -= a
    return fmt.Errorf("not enough: %d < %d", fs.food, -a)
  }
  return nil
}


func TestContract(t *testing.T) {
  cases := []struct{
    desc string
    contract *Contract
    source int
    target int
    transfer int
    err string
  } {
      {
        desc: "Insufficient",
        source: 0,
        transfer: 1,
        err: "not enough food, have 0",
      },
      {
        desc: "Works",
        source: 1,
        transfer: 1,
        target: 0,
      },
    }

  for _, cc := range cases {
    t.Run(cc.desc, func(t *testing.T) {
      lord := &testFoodStore{
        food: cc.target,
      }
      tenant := &testFoodStore{
        food: cc.source,
      }
      contract := &Contract{
        fSource: tenant,
        fTarget: lord,
        fAmount: cc.transfer,
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

      if exp := cc.target + cc.transfer; lord.food != exp {
        t.Errorf("%s: Execute() => target has %d food, expect %d", cc.desc, lord.food, exp)
      }
      if exp := cc.source - cc.transfer; tenant.food != exp {
        t.Errorf("%s: Execute() => target has %d food, expect %d", cc.desc, tenant.food, exp)
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

  ss := &Store{
    food: 0,
    wealth: 0,
  }
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
  ss = &Store{
    food: 0,
    wealth: 0,
  }
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
