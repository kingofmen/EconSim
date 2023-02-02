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

func BanditFilter(p *Pop) bool {
  return p.GetKind() == Bandit
}

func PeasantFilter(p *Pop) bool {
  return p.GetKind() == Peasant
}

func HungryFilter(p *Pop) bool {
  return p.GetHunger() > 0
}

type Pop struct {
  kind Kind
  hungry int
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

func (p *Pop) GetKind() Kind {
  if p == nil {
    return Null
  }
  return p.kind
}

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

func (p *Pop) GetHunger() int {
  if p == nil {
    return 0
  }
  return p.hungry
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

func New(k Kind) *Pop {
  return &Pop{
    kind: k,
    hungry: 0,
  }
}
