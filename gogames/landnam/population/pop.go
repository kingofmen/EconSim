// Package pop defines population dynamics.
package pop

import (
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
	errors := make([]error, 0, len(types))
	for idx, tp := range types {
		key := tp.GetKey()
		if len(key) < 1 {
			errors = append(errors, fmt.Errorf("PopType %d has zero-length key", idx))
			continue
		}
		if _, ex := mgr.types[key]; ex {
			errors = append(errors, fmt.Errorf("Duplicate key %q in PopType %d", key, idx))
			continue
		}
		mgr.types[key] = tp
	}
	return errors
}

// Validate returns any errors in the Pops.
func (mgr *Manager) Validate(pops []*poppb.Pop) []error {
	if err := mgr.check("Validate"); err != nil {
		return []error{err}
	}
	errors := make([]error, 0, len(pops))
	for idx, pop := range pops {
		key := pop.GetKind()
		if _, ex := mgr.types[key]; !ex {
			errors = append(errors, fmt.Errorf("Pop %d has unknown key %q", idx, key))
			continue
		}
	}
	return errors
}

func (mgr *Manager) Produce(pops []*poppb.Pop) error {
	if err := mgr.check("Produce"); err != nil {
		return err
	}

	for idx, pop := range pops {
		key := pop.GetKind()
		tmp, ok := mgr.types[key]
		if !ok {
			return fmt.Errorf("Produce: Pop %d of %d has bad type %q", idx, len(pops), key)
		}

		pop.Prods += tmp.GetProduction()
	}
	return nil
}
