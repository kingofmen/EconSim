// Package faction contains objects for keeping track
// of game-element loyalties.
package faction

import (
	"gogames/raubgraf/engine/dna"
	"gogames/raubgraf/engine/pop"
)

type FactionState int

const (
	Ghost FactionState = iota
	Computer
	Human
)

// Order holds an order the faction will give to a Pop.
type Order struct {
	Target uint32
	Act    pop.Action
}

// Faction models a clan descended from a male-female pair
// with uniquely identifying Y-chromosome and mitochondrial
// DNA.
type Faction struct {
	*dna.Sequence

	orders   []*Order
	state    FactionState
	capacity int
}

// New returns a new Faction.
func New(pat, mat string) *Faction {
	return &Faction{
		Sequence: dna.New(pat, mat),
		capacity: 3,
	}
}

// AddOrder adds an order to the faction's queue, removing
// an old one if needed.
func (f *Faction) AddOrder(o *Order) error {
	if f == nil {
		return nil
	}
	f.orders = append(f.orders, o)
	if len(f.orders) > f.capacity {
		f.orders = f.orders[1:]
	}
	return nil
}

func (f *Faction) ClearOrders() {
	if f == nil {
		return
	}
	f.orders = make([]*Order, 0, f.capacity)
}

func (f *Faction) GetOrders() []*Order {
	if f == nil {
		return nil
	}
	return f.orders
}

// GetDNA returns the full DNA string of the faction,
// maternal and paternal both.
func (f *Faction) GetDNA() string {
	if f == nil {
		return ""
	}
	return f.Sequence.String()
}

// Command returns true if the Faction is allowed to give orders
// to an entity with the provided DNA.
func (f *Faction) Command(seq *dna.Sequence) bool {
	if f == nil {
		return false
	}
	// Give orders only to patrilineally descended entities.
	return f.Sequence.Match(seq, dna.Paternal)
}

// IsHuman returns true if the faction is being played by a human.
func (f *Faction) IsHuman() bool {
	if f == nil {
		return false
	}
	return f.state == Human
}

// Score returns the contribution weight of an entity with
// the provided DNA to the faction's score.
func (f *Faction) Score(seq *dna.Sequence) int {
	if f == nil {
		return 0
	}
	w := 0
	if f.Sequence.Match(seq, dna.Paternal) {
		w++
	}
	if f.Sequence.Match(seq, dna.Maternal) {
		w++
	}
	return w
}
