// Package pop defines population dynamics.
package pop

import (
	"fmt"

	poppb "gogames/landnam/population/pop_go_proto"
)

// Validate returns any errors in the templates or POPs.
func Validate(templates []*poppb.PopType, pops []*poppb.Pop) []error {
	errors := make([]error, 0, len(templates))
	keys := map[string]bool{}
	for idx, tmp := range templates {
		key := tmp.GetKey()
		if len(key) < 1 {
			errors = append(errors, fmt.Errorf("PopType %d has zero-length key", idx))
			continue
		}
		if keys[key] {
			errors = append(errors, fmt.Errorf("Duplicate key %q in PopType %d", key, idx))
			continue
		}
		keys[key] = true
	}

	for idx, pop := range pops {
		key := pop.GetKind()
		if !keys[key] {
			errors = append(errors, fmt.Errorf("Pop %d has unknown key %q", idx, key))
			continue
		}
	}

	return errors
}
