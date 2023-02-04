package pop

type Kind int

const (
  Null Kind = iota
  Peasant
  Bandit
  Burgher
  Merchant
  Knight
  Noble  
)

type Filter func(p *Pop) bool

// AvailableFilter passes pops that have not acted this turn.
func AvailableFilter(p *Pop) bool {
  return !p.Busy()
}

// BanditFilter passes pops that are bandits.
func BanditFilter(p *Pop) bool {
  return p.GetKind() == Bandit
}

// HungryFilter passes pops that are hungry.
func HungryFilter(p *Pop) bool {
  return p.GetHunger() > 0
}

// NotHungryFilter passes pops that are not hungry.
func NotHungryFilter(p *Pop) bool {
  return p.GetHunger() == 0
}

// PeasantFilter passes pops that are peasants.
func PeasantFilter(p *Pop) bool {
  return p.GetKind() == Peasant
}

// Pop models a group of several dozen humans.
type Pop struct {
  kind Kind
  hungry int
  busy bool
}

// Busy returns true if the Pop has acted.
func (p *Pop) Busy() bool {
  if p == nil {
    // The nil Pop cannot do anything, so is always busy.
    return true
  }
  return p.busy
}

// Clear readies the Pop for a new turn.
func (p *Pop) Clear() {
  if p == nil {
    return
  }
  p.busy = false
}

// Copy returns a copy of the Pop.
func (p *Pop) Copy() *Pop {
  if p == nil {
    return nil
  }
  return &Pop{
    kind: p.kind,
    hungry: p.hungry,
  }
}

// Pass returns true if the pop passes all the filters.
func (p *Pop) Pass(filters ...Filter) bool {
  if p == nil {
    return false
  }
  for _, f := range filters {
    if !f(p) {
      return false
    }
  }
  return true
}

// GetKind returns the pop's kind.
func (p *Pop) GetKind() Kind {
  if p == nil {
    return Null
  }
  return p.kind
}

// Eat either resets the hunger level of the pop to zero, or increases it,
// depending on whether it's eating food or not.
func (p *Pop) Eat(food bool) {
  if p == nil {
    return
  }
  if food {
    p.hungry = 0
  } else {
    p.hungry++
  }
}

// GetHunger returns the hunger level of the pop.
func (p *Pop) GetHunger() int {
  if p == nil {
    return 0
  }
  return p.hungry
}

// Work sets the pop's busy state, returning false if it was already busy.
func (p *Pop) Work() bool {
  if p == nil {
    return false
  }
  b := p.busy
  p.busy = true
  return !b
}

// String returns a human-readable string suitable for debugging.
// It is not intended for display to users.
func (k Kind) String() string {
  switch k {
  case Null:
    return "Null"
  case Peasant:
    return "Peasant"
  case Bandit:
    return "Bandit"
  case Burgher:
    return "Burgher"
  case Merchant:
    return "Merchant"
  case Knight:
    return "Knight"
  case Noble:
    return "Noble"
  }
  return "BadKind"
}

// New returns a pop of the provided kind.
func New(k Kind) *Pop {
  return &Pop{
    kind: k,
    hungry: 0,
    busy: false,
  }
}
