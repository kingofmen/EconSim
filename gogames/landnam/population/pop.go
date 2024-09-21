// Package pop defines population dynamics.
package pop

import (
	"fmt"

	poppb "gogames/landnam/population/pop_go_proto"
)

type Manager struct {
	types map[string]*poppb.PopType
}

// WithTypes adds the types to the map, returning a slice of errors.
func (smt *Manager) WithTypes(types []*poppb.PopType) []error {
	if smt == nil {
		return []error{fmt.Errorf("nil.WithTypes")}
	}
	if smt.types == nil {
		smt.types = map[string]*poppb.PopType{}
	}
	errors := make([]error, 0, len(types))
	for idx, tp := range types {
		key := tp.GetKey()
		if len(key) < 1 {
			errors = append(errors, fmt.Errorf("PopType %d has zero-length key", idx))
			continue
		}
		if _, ex := smt.types[key]; ex {
			errors = append(errors, fmt.Errorf("Duplicate key %q in PopType %d", key, idx))
			continue
		}
		smt.types[key] = tp
	}
	return errors
}

// Validate returns any errors in the Pops.
func (smt *Manager) Validate(pops []*poppb.Pop) []error {
	if smt == nil {
		return []error{fmt.Errorf("nil.Validate")}
	}
	if smt.types == nil {
		smt.types = map[string]*poppb.PopType{}
	}
	errors := make([]error, 0, len(pops))
	for idx, pop := range pops {
		key := pop.GetKind()
		if _, ex := smt.types[key]; !ex {
			errors = append(errors, fmt.Errorf("Pop %d has unknown key %q", idx, key))
			continue
		}
	}
	return errors
}
