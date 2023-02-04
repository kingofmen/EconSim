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

func NewContract() *Contract{
  return &Contract{}
}

func (c *Contract) WithFoodSource(fs FoodStore) *Contract {
  if c == nil {
    c = NewContract()
  }
  c.fSource = fs
  return c
}

func (c *Contract) WithFoodTarget(fs FoodStore) *Contract {
  if c == nil {
    c = NewContract()
  }
  c.fTarget = fs
  return c
}

func (c *Contract) WithFoodAmount(am int) *Contract {
  if c == nil {
    c = NewContract()
  }
  c.fAmount = am
  return c
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
