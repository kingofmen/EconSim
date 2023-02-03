package econ

import (
  "fmt"
)

type FoodStore interface {
  CountFood() int
  AddFood(int) error
}

type Contract struct {
  fSource FoodStore
  fTarget FoodStore
  fAmount int
}

func (c *Contract) Execute() error {
  if c == nil {
    return nil
  }
  if c.fAmount > 0 {
    if av := c.fSource.CountFood(); av < c.fAmount {
      return fmt.Errorf("contract error: not enough food, have %d want %d", av, c.fAmount)
    }
    if err := c.fSource.AddFood(-c.fAmount); err != nil {
      return fmt.Errorf("contract error in source AddFood(): %w", err)
    }
    if err := c.fTarget.AddFood(c.fAmount); err != nil {
      return fmt.Errorf("contract error in target AddFood(): %w", err)
    }
  }

  return nil
}
