package econ

import (
  "fmt"
)

type Resource int

const (
  Food Resource = iota
  Wealth
)

type Store struct {
  food int
  wealth int
}

// String returns a human-readable string suitable for debugging.
// It is not intended to be seen by the user.
func (r Resource) String() string {
  switch r {
  case Food:
    return "Food"
  case Wealth:
    return "Wealth"
  }
  return "Unknown resource"
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
  Source *Store
  Target *Store
  Amount *Store
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

func (c *Contract) WithSource(fs *Store) *Contract {
  if c == nil {
    c = NewContract()
  }
  c.Source = fs
  return c
}

func (c *Contract) WithTarget(fs *Store) *Contract {
  if c == nil {
    c = NewContract()
  }
  c.Target = fs
  return c
}

func (c *Contract) WithAmount(am *Store) *Contract {
  if c == nil {
    c = NewContract()
  }
  c.Amount = am
  return c
}

func (c *Contract) Execute() error {
  if c == nil {
    return nil
  }

  for _, res := range []Resource{Food, Wealth} {
    amount := c.Amount.Count(res)
    if amount == 0 {
      continue
    }
    available := c.Source.Count(res)
    if amount > available {
      return fmt.Errorf("contract error: not enough %s, have %d want %d", res, available, amount)
    }
    if err := c.Source.AddResource(res, -amount); err != nil {
      return fmt.Errorf("contract error in source AddResource(%s): %w", res, err)
    }
    if err := c.Target.AddResource(res, amount); err != nil {
      return fmt.Errorf("contract error in target AddResource(%s): %w", res, err)
    }
  }

  return nil
}
