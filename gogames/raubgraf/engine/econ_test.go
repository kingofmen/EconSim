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
