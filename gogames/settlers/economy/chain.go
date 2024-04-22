// Package chain implements production networks.
package chain

import (
	"fmt"

	"google.golang.org/protobuf/proto"

	cpb "gogames/settlers/economy/chain_proto"
)

// Work models a production process at a particular location.
type Work struct {
	// Work type.
	key string
	// Labour and capital used.
	assigned []*cpb.Level
}

// scalable returns the level of the process that can be
// stacked onto this box, if there is one.
func (w *Work) scalable(p *cpb.Process) int {
	if w == nil {
		return -1
	}
	if w.key != p.GetKey() {
		return -1
	}
	cand := len(w.assigned)
	if cand >= len(p.GetLevels()) {
		return -1
	}
	return cand
}

type Scorer interface {
	Score(exist, marginal map[string]int32) int32
}

// ScorerFunc returns a score for the given basket.
type ScorerFunc func(exist, marginal map[string]int32) int32

func (sf ScorerFunc) Score(exist, marginal map[string]int32) int32 {
	return sf(exist, marginal)
}

// Location represents an area containing 'boxes' that can be
// filled with work processes; it is small enough that all the
// boxen can communicate with negligible frictional costs.
type Location struct {
	// Filled boxes.
	boxen []*Work
	// Labour pool.
	pool map[string]int32
	// Connected locations.
	network []*Location
	// Prioritiser.
	evaluator Scorer
	// Stockpile.
	goods map[string]int32
	// Max work processes.
	maxBox int
}

// super returns true if p1 is a superset of p2.
func super(avail, want map[string]int32) bool {
	for skill, amount := range want {
		if avail[skill] < amount {
			return false
		}
	}
	return true
}

// placement is a possible scaling of a process.
type placement struct {
	pos int
	lvl int
}

// Allowed returns the possible placements of the given process.
func (loc *Location) Allowed(p *cpb.Process) []*placement {
	// TODO: Constraints other than number of workers.
	places := make([]*placement, 0, 3)
	// First check scaling.
	for idx, box := range loc.boxen {
		lvl := box.scalable(p)
		if lvl < 0 {
			continue
		}
		need := p.GetLevels()[lvl]
		if !super(loc.pool, need.GetWorkers()) {
			continue
		}
		// Other prereqs must be fulfilled since we have
		// a box with it already.
		places = append(places, &placement{
			pos: idx,
			lvl: lvl,
		})
	}
	if len(loc.boxen) < loc.maxBox {
		if super(loc.pool, p.GetLevels()[0].GetWorkers()) {
			places = append(places, &placement{
				pos: -1,
				lvl: 0,
			})
		}
	}
	return places
}

// output returns what this location will produce by adding the
// process in place
func (loc *Location) output(place *placement, p *cpb.Process) map[string]int32 {
	return p.GetLevels()[place.lvl].GetOutputs()
}

// defaultScore returns a score prioritising all goods equally
// with no diminishing returns.
func defaultScore(exist, output map[string]int32) int32 {
	ret := int32(0)
	for _, amt := range output {
		ret += amt
	}
	return ret
}

// score returns the priority of the process.
func (loc *Location) score(place *placement, proc *cpb.Process) int32 {
	output := loc.output(place, proc)
	if loc.evaluator != nil {
		return loc.evaluator.Score(loc.goods, output)
	}
	return defaultScore(loc.goods, output)
}

// assignWork fills a box with the provided process.
func (loc *Location) assignWork(pos int, p *cpb.Process) {
	var box *Work
	if pos >= 0 {
		box = loc.boxen[pos]
	} else {
		box = &Work{
			key: p.GetKey(),
		}
		loc.boxen = append(loc.boxen, box)
	}
	lvl := proto.Clone(p.GetLevels()[len(box.assigned)]).(*cpb.Level)
	box.assigned = append(box.assigned, lvl)
	for skill, amount := range lvl.GetWorkers() {
		loc.pool[skill] -= amount
	}
}

// Place assigns labour to the best available Process.
// It returns the key of the process chosen.
func (loc *Location) Place(options []*cpb.Process) string {
	legal := make(map[*cpb.Process][]*placement)
	for _, opt := range options {
		if ps := loc.Allowed(opt); len(ps) > 0 {
			legal[opt] = ps
		}
	}
	if len(legal) == 0 {
		return ""
	}

	var best *cpb.Process
	pts := int32(-1000)
	pos := -1000
	for proc, places := range legal {
		for _, ps := range places {
			if curr := loc.score(ps, proc); curr > pts {
				pts = curr
				best = proc
				pos = ps.pos
			}
		}
	}
	if pts < 0 {
		return ""
	}

	loc.assignWork(pos, best)
	return best.GetKey()
}

func Valid(web *cpb.Web) error {
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
