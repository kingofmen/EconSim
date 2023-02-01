package pop

import (
  "testing"
)

func TestHunger(t *testing.T) {
  pp := New(Peasant)
  cases := []struct{
    food bool
    exp int
  } {
      {
        food: false,
        exp: 1,
      },
      {
        food: false,
        exp: 2,
      },
      {
        food: false,
        exp: 3,
      },
      {
        food: true,
        exp: 0,
      },
      {
        food: false,
        exp: 1,
      },
    }

  for idx, cc := range cases {
    pp.Eat(cc.food)
    if got := pp.GetHunger(); got != cc.exp {
      t.Errorf("TestHunger(%d): GetHunger() -> %d, want %d", idx, got, cc.exp)
    }
  }
}
