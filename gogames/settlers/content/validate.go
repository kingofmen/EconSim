// Package validate contains validation methods for content.
package validate

import (
	"fmt"

	cpb "gogames/settlers/economy/chain_proto"
)

// Web returns an error if the web is invalid.
func Web(web *cpb.Web) error {
	nodes := web.GetNodes()
	if len(nodes) < 1 {
		return fmt.Errorf("Web must have at least one node")
	}
	fnode := nodes[0]
	nlvl := len(fnode.GetLevels())
	if nlvl < 1 {
		return fmt.Errorf("Processes must have at least one level - %q has %d", fnode.GetKey(), nlvl)
	}
	if len(fnode.GetKey()) < 1 {
		return fmt.Errorf("Processes must have non-empty keys")
	}

	seen := map[string]bool{fnode.GetKey(): true}
	for _, n := range nodes[1:] {
		key := n.GetKey()
		if len(key) == 0 {
			return fmt.Errorf("Processes must have non-empty keys")
		}
		if seen[key] {
			return fmt.Errorf("Process keys must be unique, %q occurs twice", key)
		}
		seen[key] = true
		if nl := len(n.GetLevels()); nl != nlvl {
			return fmt.Errorf("Processes in web must have same number of levels - %q has %d versus %q with %d", n.GetKey(), nl, fnode.GetKey(), nlvl)
		}
	}
	if ntl := len(web.GetTransport()); ntl != nlvl {
		return fmt.Errorf("Web must have same number of transport and production levels - found %d transport, %d production (in %q)", ntl, nlvl, fnode.GetKey())
	}
	return nil
}
