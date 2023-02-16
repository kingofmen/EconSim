package econ

import (
  "fmt"
)

type Resource int

const (
  Food Resource = iota
  Wealth
)

type FoodStore interface {
  CountFood() int
  AddFood(int) error
}

type Store struct {
  food int
  wealth int
}

// AddFood adds the provided amount of food to the store,
// which may be negative. An error is returned if the store
// would end up with negative food.
func (s *Store) AddFood(am int) error {
  if s == nil {
    return nil
  }
  s.food += am
  if s.food < 0 {
    err := fmt.Errorf("attempt to subtract %d food from stockpile %d", -am, s.food - am)
    s.food = 0
    return err
  }

  return nil  
}

// CountFood returns the amount of stored food.
func (s *Store) CountFood() int {
  if s == nil {
    return 0
  }
  return s.food
}

// AddWealth adds the provided amount of wealth to the store,
// which may be negative. An error is returned if the store
// would end up with negative wealth.
func (s *Store) AddWealth(am int) error {
  if s == nil {
    return nil
  }
  s.wealth += am
  if s.wealth < 0 {
    err := fmt.Errorf("attempt to subtract %d wealth from stockpile %d", -am, s.wealth - am)
    s.wealth = 0
    return err
  }

  return nil  
}

// CountWealth returns the amount of stored wealth.
func (s *Store) CountWealth() int {
  if s == nil {
    return 0
  }
  return s.wealth
}

// Add adds the provided amount of food or wealth to the store,
// which may be negative. An error is returned if the stockpile
// would end up negative.
func (s *Store) AddResource(r Resource, am int) error {
  if r == Food {
    return s.AddFood(am)
  }
  if r == Wealth {
    return s.AddWealth(am)
  }
  return fmt.Errorf("unknown resource %s (%d) supplied to Add", r, am)
}

// Count returns the amount of stored food or wealth.
func (s *Store) Count(r Resource) int {
  if r == Food {
    return s.CountFood()
  }
  if r == Wealth {
    return s.CountWealth()
  }
  return 0
}

type Contract struct {
  fSource FoodStore
  fTarget FoodStore
  fAmount int
}

// NewStore returns an empty Store.
func NewStore() *Store {
  return &Store{
    food: 0,
    wealth: 0,
  }
}

// NewContract returns a blank Contract.
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
