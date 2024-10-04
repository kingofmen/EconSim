// Package tables defines an engine for resolving battles
// based on data-driven states.
package tables

import (
	"fmt"

	"gogames/util/logic"

	tpb "gogames/combat/tables_go_proto"
)

type Lookup struct {
	states map[string]*tpb.Phase
}

// NewLookup returns a new Lookup object.
func NewLookup() *Lookup {
	return &Lookup{
		states: make(map[string]*tpb.Phase),
	}
}

// AddPhases adds the phases to the lookup.
func (l *Lookup) AddPhases(phases []*tpb.Phase) []error {
	if l == nil {
		return []error{fmt.Errorf("AddPhases called on nil tables.Lookup")}
	}
	errors := make([]error, 0, len(phases))
	if l.states == nil {
		l.states = make(map[string]*tpb.Phase)
	}
	for idx, ph := range phases {
		key := ph.GetKey()
		if len(key) == 0 {
			errors = append(errors, fmt.Errorf("phase %d has empty key", idx))
			continue
		}
		if _, exists := l.states[key]; exists {
			errors = append(errors, fmt.Errorf("duplicate key %q in phase %d", key, idx))
			continue
		}
		l.states[key] = ph
	}

	hasEnd := false
	for _, state := range l.states {
		key := state.GetKey()
		if len(state.GetResults()) == 0 {
			errors = append(errors, fmt.Errorf("phase %q has no results", key))
		}
		for _, res := range state.GetResults() {
			name := state.GetKey()
			next := res.GetGoto()
			if res.GetEnds() {
				hasEnd = true
				if len(next) > 0 {
					errors = append(errors, fmt.Errorf("result %q in phase %q both ends and goes to %q", name, key, next))
				}
			}
			if len(next) > 0 {
				if _, exists := l.states[next]; !exists {
					errors = append(errors, fmt.Errorf("result %q in phase %q goes to nonexistent %q", name, key, next))
				}
			}
		}
	}
	if !hasEnd {
		errors = append(errors, fmt.Errorf("no ending state found"))
	}

	return errors
}

func (l *Lookup) Resolve(key string, lookup logic.Lookup) (map[string]int32, error) {
	_, ok := l.states[key]
	if !ok {
		return nil, fmt.Errorf("unknown starting state %q", key)
	}
	return nil, nil
}
