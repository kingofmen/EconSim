package pop

import(
  "gogames/raubgraf/engine/war"
)

type Kind int
type Action int

const (
  Null Kind = iota
  Peasant
  Bandit
  Burgher
  Merchant
  Knight
  Noble

  Unknown Action = iota
  Banditry
  Labour
  Levy
)

type Filter func(p *Pop) bool
type List []*Pop
type Dwelling struct {
  pops List
}

// AvailableFilter passes pops that have not acted this turn.
func AvailableFilter(p *Pop) bool {
  return !p.Busy()
}

// BanditFilter passes pops that are bandits.
func BanditFilter(p *Pop) bool {
  return p.GetKind() == Bandit
}

// BusyFilter passes pops that cannot act.
func BusyFilter(p *Pop) bool {
  return p.Busy()
}

// FightingFilter passes pops that are able to fight.
func FightingFilter(p *Pop) bool {
  if p == nil {
    return false
  }

  if p.Busy() {
    return false
  }

  switch p.GetKind() {
  case Peasant:
    // Peasants fight only if they are raised as militia.
    return p.action == Levy
  default:
    // All other pops fight at all times if they can.
    // TODO: Probably not!
    return true
  }

  return false
}

// HungryFilter passes pops that are hungry.
func HungryFilter(p *Pop) bool {
  return p.GetHunger() > 0
}

// KindsFilter returns a filter that passes the given kinds.
func KindsFilter(kinds ...Kind) Filter {
  return func(p *Pop) bool {
    for _, k := range kinds {
      if p.GetKind() == k {
        return true
      }
    }
    return false
  }
}

// KnightFilter passes pops that are knights.
func KnightFilter(p *Pop) bool {
  return p.GetKind() == Knight
}

// NotHungryFilter passes pops that are not hungry.
func NotHungryFilter(p *Pop) bool {
  return p.GetHunger() == 0
}

// PeasantFilter passes pops that are peasants.
func PeasantFilter(p *Pop) bool {
  return p.GetKind() == Peasant
}

// LabourFilter passes pops currently working.
func LabourFilter(p *Pop) bool {
  return p.Activity() == Labour
}

// NewDwelling returns an empty Dwelling object.
func NewDwelling() *Dwelling {
  return &Dwelling{
    pops: List{},
  }
}

// Count returns the number of Pops that pass the provided filters.
func (l List) Count(filters ...Filter) int {
  count := 0
  for _, pp := range l {
    if pp.Pass(filters...) {
      count++
    }
  }
  return count
}

// Filter returns those Pops that pass the provided filters.
func (l List) Filter(filters ...Filter) List {
  ret := make(List, 0, len(l))
  for _, pp := range l {
    if pp.Pass(filters...) {
      ret = append(ret, pp)
    }
  }
  return ret;
}

// First returns the first Pop that passes the filters, if any.
func (l List) First(filters ...Filter) *Pop {
  for _, pp := range l {
    if pp.Pass(filters...) {
      return pp
    }
  }
  return nil
}

// AddPop adds the provided pop to the population.
func (d *Dwelling) AddPop(p *Pop) {
  if d == nil {
    return
  }
  if p == nil {
    return
  }
  d.pops = append(d.pops, p)
}

// CountKind returns the number of k-type Pops.
func (d *Dwelling) CountKind(k Kind) int {
  return d.CountPops(func(p *Pop) bool {
    return p.GetKind() == k
  })
}

// CountPops returns the number of pops that pass all the filters.
func (d *Dwelling) CountPops(filters ...Filter) int {
  if d == nil {
    return 0
  }
  return d.pops.Count(filters...)
}

// FirstPop returns the first Pop that passes the filters, if any.
func (d *Dwelling) FirstPop(filters ...Filter) *Pop {
  if d == nil {
    return nil
  }
  return d.pops.First(filters...)
}

// FilteredPopulation returns a slice of the Pops that pass the filters.
func (d *Dwelling) FilteredPopulation(filters ...Filter) List {
  if d == nil {
    return nil
  }
  return d.pops.Filter(filters...)
}

// Population returns a slice of the Pops.
func (d *Dwelling) Population() List {
  if d == nil {
    return nil
  }
  ret := make(List, 0, len(d.pops))
  ret = append(ret, d.pops...)
  return ret;
}

// RemoveKind removes and returns the first Pop of the given kind.
func (d *Dwelling) RemoveKind(k Kind) *Pop {
  if d == nil {
    return nil
  }
  for i := len(d.pops) - 1; i >= 0; i-- {
    curr := d.pops[i]
    if curr.GetKind() != k {
      continue
    }
    d.pops = append(d.pops[:i], d.pops[i+1:]...)
    return curr
  }
  return nil
}

// RemovePop removes the provided pop, returning it if found.
func (d *Dwelling) RemovePop(p *Pop) *Pop {
  if d == nil {
    return nil
  }

  for i := len(d.pops) - 1; i >= 0; i-- {
    curr := d.pops[i]
    if curr != p {
      continue
    }
    d.pops = append(d.pops[:i], d.pops[i+1:]...)
    return curr
  }

  return nil
}

// Pop models a group of several dozen humans.
type Pop struct {
  kind Kind
  hungry int
  busy bool
  action Action

  levy *war.FieldUnit
}

func (p *Pop) Activity() Action {
  if p == nil {
    return Unknown
  }
  return p.action
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
  p.action = defaultAction(p.kind)
}

// defaultAction returns the default activity for the pop type.
func defaultAction(k Kind) Action {
  switch k {
  case Peasant:
    return Labour
  case Bandit:
    return Banditry
  case Knight:
    return Levy
  default:
    return Unknown
  }
  return Unknown  
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

// GetHunger returns the hunger level of the Pop.
func (p *Pop) GetHunger() int {
  if p == nil {
    return 0
  }
  return p.hungry
}

// SetAction sets the pop's activity for the turn.
func (p *Pop) SetAction(act Action) {
  if p == nil {
    return
  }
  p.action = act
}

// Mobilise returns the pop's levy.
func (p *Pop) Mobilise() *war.FieldUnit {
  if p == nil {
    return nil
  }
  // TODO: Add WithFoo calls depending on Pop state.
  if p.GetKind() == Knight {
    // TODO: This is a placeholder for an Actual Model of mobilisation.
    p.levy = p.levy.WithMount(6).WithDrill(1)
  }
  return p.levy
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
    levy: war.NewUnit(),
  }
}
