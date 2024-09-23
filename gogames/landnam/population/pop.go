// Package pop defines population dynamics.
package pop

import (
	"errors"
	"fmt"

	poppb "gogames/landnam/population/pop_go_proto"
)

type Manager struct {
	types map[string]*poppb.PopType
}

func NewManager() *Manager {
	return &Manager{
		types: map[string]*poppb.PopType{},
	}
}

func (mgr *Manager) check(method string) error {
	if mgr == nil {
		return fmt.Errorf("%q called on nil Manager")
	}
	if mgr.types == nil {
		mgr.types = map[string]*poppb.PopType{}
	}
	return nil
}

// WithTypes adds the types to the map, returning a slice of errors.
func (mgr *Manager) WithTypes(types []*poppb.PopType) []error {
	if err := mgr.check("WithTypes"); err != nil {
		return []error{err}
	}
	errs := make([]error, 0, len(types))
	for idx, tp := range types {
		key := tp.GetKey()
		if len(key) < 1 {
			errs = append(errs, fmt.Errorf("PopType %d has zero-length key", idx))
			continue
		}
		if _, ex := mgr.types[key]; ex {
			errs = append(errs, fmt.Errorf("Duplicate key %q in PopType %d", key, idx))
			continue
		}
		mgr.types[key] = tp
	}
	return errs
}

func (mgr *Manager) popsExist(pops []*poppb.Pop) []error {
	errs := make([]error, 0, len(pops))
	for idx, pop := range pops {
		key := pop.GetKind()
		if _, ex := mgr.types[key]; !ex {
			errs = append(errs, fmt.Errorf("Pop %d has unknown key %q", idx, key))
			continue
		}
	}
	return errs
}

// Validate returns any errors in the Pops.
func (mgr *Manager) Validate(pops []*poppb.Pop) []error {
	if err := mgr.check("Validate"); err != nil {
		return []error{err}
	}
	errs := mgr.popsExist(pops)
	return errs
}

func (mgr *Manager) Produce(pops []*poppb.Pop) error {
	if err := mgr.check("Produce"); err != nil {
		return err
	}
	if errs := mgr.popsExist(pops); len(errs) > 0 {
		return errors.Join(errs...)
	}

	for _, pop := range pops {
		pop.Prods += mgr.types[pop.GetKind()].GetProduce()
	}
	return nil
}

// minimumFood consumes enough prods to get the POP up to its minimum,
// if it's available, and returns the amount of prods remaining.
func minimumFood(tmp *poppb.PopType, pop *poppb.Pop) int32 {
	avail := pop.GetProds()
	req := tmp.GetMinConsume() - pop.GetConsume()
	if req <= 0 {
		return avail
	}
	if avail > req {
		avail = req
	}
	pop.Prods -= avail
	pop.Consume += avail
	return pop.GetProds()
}

// sizeModifier returns the additional cost of a level of
// consumption due to POP size.
func sizeModifier(tmp *poppb.PopType, pop *poppb.Pop) int32 {
	return 0
}

// use consumes the amount of prods given by the want function
// (if they exist, otherwise the amount available), and stores
// them in the target field. It returns the amount consumed.
func use(tmp *poppb.PopType, pop *poppb.Pop, want func() int32, target *int32, mods ...func(*poppb.PopType, *poppb.Pop) int32) int32 {
	req := want()
	for _, m := range mods {
		req += m(tmp, pop)
	}
	avail := pop.GetProds()
	if avail > req {
		avail = req
	}
	pop.Prods -= avail
	*target += avail
	return avail
}

// Consume iterates over the POPs and distributes their available
// production to their priorities.
func (mgr *Manager) Consume(pops []*poppb.Pop) error {
	if err := mgr.check("Consume"); err != nil {
		return err
	}
	if errs := mgr.popsExist(pops); len(errs) > 0 {
		return errors.Join(errs...)
	}

	for _, pop := range pops {
		tmp := mgr.types[pop.GetKind()]
		if minimumFood(tmp, pop) < 1 {
			continue
		}

		for pop.GetProds() > 0 {
			used := use(tmp, pop, tmp.GetConsume, &(pop.Consume), sizeModifier)
			used += use(tmp, pop, tmp.GetCapital, &(pop.Capital))
			used += use(tmp, pop, tmp.GetMilitia, &(pop.Militia))
			used += use(tmp, pop, tmp.GetMeaning, &(pop.Meaning))
			if used < 1 {
				break
			}
		}
	}
	return nil
}
