// Package tables defines an engine for resolving battles
// based on data-driven states.
package tables

import (
	"fmt"

	"gogames/util/logic"

	tpb "gogames/combat/tables_go_proto"
)

// DieRoller encapsulates a random-number source.
type DieRoller interface {
	Roll(face int32) int32
}

// Lookup gives the result of a set of Phases.
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
		if len(ph.GetRolls()) < 1 {
			errors = append(errors, fmt.Errorf("phase %q has no dierolls", key))
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
			if p := res.GetPips(); p < 1 {
				errors = append(errors, fmt.Errorf("result %q in phase %q has fewer pips than 1", name, key))
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

// rollDice returns the unmodified result of rolling the dice for the phase.
func rollDice(phase *tpb.Phase, dice DieRoller) int32 {
	var roll int32
	for _, r := range phase.GetRolls() {
		roll += dice.Roll(r)
	}
	return roll
}

// getModifiers returns the sum of the applicable modifiers for the phease.
func getModifiers(phase *tpb.Phase, lookup logic.Lookup) ([]*tpb.Modifier, int32, error) {
	var mods []*tpb.Modifier
	var value int32
	for _, mod := range phase.GetModifiers() {
		allow := true
		for idx, req := range mod.GetRequires() {
			ok, err := logic.Eval(req, lookup)
			if err != nil {
				return nil, 0, fmt.Errorf("error evaluating requirement %d of modifier %q in phase %q: %w", idx, mod.GetKey(), phase.GetKey(), err)
			}
			if ok {
				continue
			}
			allow = false
			break
		}
		if !allow {
			continue
		}
		mods = append(mods, mod)
		value += mod.GetValue()
	}
	return mods, value, nil
}

// getOutcome returns the phase result indicated by the roll.
func getOutcome(roll int32, phase *tpb.Phase, lookup logic.Lookup) (*tpb.Result, error) {
	var accum int32
	var outcome *tpb.Result
	for _, result := range phase.GetResults() {
		allow := true
		for idx, req := range result.GetRequires() {
			ok, err := logic.Eval(req, lookup)
			if err != nil {
				return nil, fmt.Errorf("error evaluating requirement %d of result %q in phase %q: %w", idx, result.GetKey(), phase.GetKey(), err)
			}
			if ok {
				continue
			}
			allow = false
			break
		}
		if !allow {
			continue
		}
		if accum += result.GetPips(); accum < roll {
			continue
		}
		outcome = result
		break
	}
	if accum < 1 {
		return nil, fmt.Errorf("phase %q has %d results none of which are allowed", phase.GetKey(), len(phase.GetResults()))
	}
	return outcome, nil
}

func (l *Lookup) Resolve(key string, dice DieRoller, lookup logic.Lookup) (*tpb.Outcome, error) {
	phase, ok := l.states[key]
	if !ok {
		return nil, fmt.Errorf("unknown starting state %q", key)
	}

	maxIters := 10000
	var iter int
	fullResult := &tpb.Outcome{
		Flags: make(map[string]int32),
	}
	for {
		pName := phase.GetKey()
		if iter++; iter > maxIters {
			return nil, fmt.Errorf("table entry %q not resolved after %d iterations; final phase was %q", key, iter, pName)
		}
		roll := rollDice(phase, dice)
		mods, add, err := getModifiers(phase, lookup)
		if err != nil {
			return nil, err
		}
		outcome, err := getOutcome(roll+add, phase, lookup)
		if err != nil {
			return nil, err
		}
		report := &tpb.Result{
			Key:  outcome.GetKey(),
			Pips: roll,
		}
		fullResult.Phases = append(fullResult.Phases, &tpb.Phase{
			Key:       pName,
			Modifiers: make([]*tpb.Modifier, 0, len(mods)),
			Results:   []*tpb.Result{report},
		})
		for _, mod := range mods {
			report.Flags = append(report.Flags, &tpb.Modifier{
				Key:   mod.GetKey(),
				Value: mod.GetValue(),
			})
		}
		if outcome == nil {
			continue
		}
		for idx, f := range outcome.GetFlags() {
			apply := true
			for _, req := range f.GetRequires() {
				match, err := logic.Eval(req, lookup)
				if err != nil {
					return nil, fmt.Errorf("error evaluating flag %d in outcome %q of phase %q: %w", idx, outcome.GetKey(), phase.GetKey(), err)
				}
				if !match {
					apply = false
					break
				}
			}
			if !apply {
				continue
			}
			fullResult.Flags[f.GetKey()] += f.GetValue()
		}

		if outcome.GetEnds() {
			break
		}
		// Error here is prevented by AddPhases validation.
		phase = l.states[outcome.GetGoto()]
	}

	return fullResult, nil
}
