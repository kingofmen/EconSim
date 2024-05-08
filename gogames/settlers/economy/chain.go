// Package chain implements production networks.
package chain

import (
	"fmt"
	"slices"
	"strings"

	"google.golang.org/protobuf/proto"

	cpb "gogames/settlers/economy/chain_proto"
)

const (
	// kNewBox indicates that a previously-unfilled box will be used.
	kNewBox = -1
	// kNotScalable indicates that the work cannot be scaled.
	kNotScalable = -2
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
		return kNewBox
	}
	if w.key != p.GetKey() {
		return kNotScalable
	}
	cand := len(w.assigned)
	if cand >= len(p.GetLevels()) {
		return kNotScalable
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
	loc *Location
	prc *cpb.Process
	pos int
	lvl int
}

// combo is a set of placements comprising a web.
type combo struct {
	// key is the lookup key of the web.
	key string
	// procs are the process placements.
	procs []*placement
}

// debugString returns a human-readable-ish string suitable
// for debugging.
func (c *combo) debugString() string {
	if c == nil {
		return "<nil>"
	}
	strs := make([]string, 0, len(c.procs))
	for _, p := range c.procs {
		strs = append(strs, fmt.Sprintf(" {loc: %p prc: %s pos: %d lvl: %d}\n", p.loc, p.prc.GetKey(), p.pos, p.lvl))
	}
	return fmt.Sprintf("%s: %s", c.key, strings.Join(strs, " "))
}

// Allowed returns the possible placements of the given process,
// taking into account any previous placements in the location.
func (loc *Location) Allowed(p *cpb.Process, prev ...*placement) []*placement {
	// TODO: Constraints other than number of workers.
	places := make([]*placement, 0, 3)
	avail := map[string]int32{}
	filled := 0
	for k, v := range loc.pool {
		avail[k] = v
	}
	for _, pl := range prev {
		if pl.loc != loc {
			continue
		}
		if pl.pos == kNewBox {
			filled++
		}
		for k, v := range pl.prc.GetLevels()[pl.lvl].GetWorkers() {
			avail[k] -= v
		}
	}
	open := 0
	// First check scaling.
	for idx, box := range loc.boxen {
		// TODO: Allow empty boxen.
		lvl := box.scalable(p)
		if lvl == kNotScalable {
			continue
		}
		if lvl == kNewBox {
			open++
			continue
		}
		need := p.GetLevels()[lvl]
		if !super(avail, need.GetWorkers()) {
			continue
		}
		// Other prereqs must be fulfilled since we have
		// a box with it already.
		places = append(places, &placement{
			loc: loc,
			prc: p,
			pos: idx,
			lvl: lvl,
		})
	}

	if len(loc.boxen)+filled-open < loc.maxBox {
		if super(avail, p.GetLevels()[0].GetWorkers()) {
			places = append(places, &placement{
				loc: loc,
				prc: p,
				pos: kNewBox,
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
func (loc *Location) score(place *placement) int32 {
	output := loc.output(place, place.prc)
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
			if curr := loc.score(ps); curr > pts {
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

// generate finds all legal combinations of locations to place the web.
func generate(web *cpb.Web, locs []*Location) []*combo {
	if web == nil {
		return nil
	}
	nodes := web.GetNodes()
	fnode := nodes[0]
	combos := make([]*combo, 0, len(locs))
	for _, loc := range locs {
		for _, cand := range loc.Allowed(fnode) {
			combos = append(combos, &combo{
				key:   web.GetKey(),
				procs: []*placement{cand},
			})
		}
	}

	for _, proc := range nodes[1:] {
		if len(combos) == 0 {
			break
		}

		// For each existing list of placements, either grow it
		// by adding a new location (possibly forking), or abandon
		// it if no additions are possible.
		ncombs := make([]*combo, 0, len(combos))
		for _, cmb := range combos {
			start := cmb.procs[len(cmb.procs)-1].loc
			for _, loc := range start.network {
				for _, cand := range loc.Allowed(proc, cmb.procs...) {
					nc := &combo{
						key:   web.GetKey(),
						procs: make([]*placement, len(cmb.procs)),
					}
					copy(nc.procs, cmb.procs)
					nc.procs = append(nc.procs, cand)
					ncombs = append(ncombs, nc)
				}
			}
		}
		combos = ncombs
	}

	return combos
}

func instantRunoff(combos []*combo, scores map[*Location]map[*combo]int32) *combo {
	for {
		votes := make(map[*combo]int)
		for loc, locScores := range scores {
			// Sort by the location's score and vote for highest score.
			slices.SortFunc(combos, func(a, b *combo) int {
				sca, scb := locScores[a], locScores[b]
				// For ascending order return negative when a < b;
				// this reverses that to get descending order.
				return int(scb - sca)
			})
			if cand := combos[0]; scores[loc][cand] > 0 {
				votes[cand]++
			}
		}
		if len(votes) == 0 {
			return nil
		}

		// Sort by votes (ascending).
		slices.SortFunc(combos, func(a, b *combo) int {
			return votes[a] - votes[b]
		})

		frst, last := combos[0], combos[len(combos)-1]
		if votes[frst] == votes[last] {
			// All are equal; pick one at random.
			// TODO: Use random generator instead of relying
			// on sort instability.
			return frst
		}
		prev := votes[frst]
		for idx, cmb := range combos[1:] {
			if votes[cmb] == prev {
				continue
			}
			combos = combos[idx+1:]
			break
		}
	}
}
