// Package chain implements production networks.
package chain

import (
	"fmt"
	"maps"
	"slices"
	"strings"

	"gogames/util/counts"
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
	// Currently active level.
	active int
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
	Goods map[string]int32
	// Max work processes.
	maxBox int
	// Rejection reasons.
	Reasons map[string]string
	// Statistics.
	Produced map[string]int32
	Consumed map[string]int32
}

func NewLocation() *Location {
	return &Location{
		pool:     make(map[string]int32),
		Goods:    make(map[string]int32),
		Produced: make(map[string]int32),
		Consumed: make(map[string]int32),
		Reasons:  make(map[string]string),
		maxBox:   10,
	}
}

// reject stores the reason for not allowing the process.
func (loc *Location) reject(p *cpb.Process, reason string) {
	if loc == nil || loc.Reasons == nil {
		return
	}
	loc.Reasons[p.GetKey()] = reason
}

func (l *Location) WithEvaluator(s Scorer) *Location {
	if l != nil {
		l.evaluator = s
	}
	return l
}

func (l *Location) WithMaxBox(mb int) *Location {
	if l != nil {
		l.maxBox = mb
	}
	return l
}

// MakeAvailable distributes the given skills among the
// Location's work boxen, leaving any surplus in the pool.
func (l *Location) MakeAvailable(work map[string]int32) {
	if l == nil {
		return
	}
	for _, b := range l.boxen {
		if b == nil {
			continue
		}
		b.active = -1
		for idx, lvl := range b.assigned {
			if !counts.SuperInt32(work, lvl.GetWorkers()) {
				continue
			}
			counts.SubtractInt32(work, lvl.GetWorkers())
			b.active = idx
		}
	}
	l.pool = maps.Clone(work)
}

func (l *Location) Work() {
	if l == nil {
		return
	}
	l.Produced = map[string]int32{}
	for _, b := range l.boxen {
		if b == nil {
			continue
		}
		for _, lvl := range b.assigned[:b.active+1] {
			for k, v := range lvl.GetOutputs() {
				l.Goods[k] += v
				l.Produced[k] += v
			}
		}
	}
}

// placement is a possible scaling of a process.
type placement struct {
	loc *Location
	prc *cpb.Process
	pos int
	lvl int
}

// Combo is a set of placements comprising a web.
type Combo struct {
	// key is the lookup key of the web.
	key string
	// procs are the process placements.
	procs []*placement
}

// DebugString returns a human-readable-ish string suitable
// for debugging.
func (c *Combo) DebugString() string {
	if c == nil {
		return "<nil>"
	}
	strs := make([]string, 0, len(c.procs))
	for _, p := range c.procs {
		strs = append(strs, fmt.Sprintf(" {loc: %p prc: %s pos: %d lvl: %d}\n", p.loc, p.prc.GetKey(), p.pos, p.lvl))
	}
	return fmt.Sprintf("%s: %s", c.key, strings.Join(strs, " "))
}

// Activate emplaces the combination's processes in the locations.
func (c *Combo) Activate() {
	if c == nil {
		return
	}
	for _, pl := range c.procs {
		pl.loc.assignWork(pl.pos, pl.prc)
	}
}

// Allowed returns the possible placements of the given process,
// taking into account any previous placements in the location.
func (loc *Location) Allowed(p *cpb.Process, prev ...*placement) []*placement {
	if p == nil {
		return nil
	}
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
			loc.reject(p, "unscalable")
			continue
		}
		if lvl == kNewBox {
			open++
			continue
		}
		need := p.GetLevels()[lvl]
		if !counts.SuperInt32(avail, need.GetWorkers()) {
			loc.reject(p, "insufficient workers")
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
		if counts.SuperInt32(avail, p.GetLevels()[0].GetWorkers()) {
			places = append(places, &placement{
				loc: loc,
				prc: p,
				pos: kNewBox,
				lvl: 0,
			})
		} else {
			loc.reject(p, fmt.Sprintf("not enough workers (%v vs %v)", avail, p.GetLevels()[0].GetWorkers()))
		}
	} else {
		loc.reject(p, "no room")
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
		return loc.evaluator.Score(loc.Goods, output)
	}
	return defaultScore(loc.Goods, output)
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

// generate finds all legal combinations of locations to place the web.
func generate(web *cpb.Web, locs []*Location) []*Combo {
	if web == nil {
		return nil
	}
	nodes := web.GetNodes()
	fnode := nodes[0]
	combos := make([]*Combo, 0, len(locs))
	for i, loc := range locs {
		for j, cand := range loc.Allowed(fnode) {
			combos = append(combos, &Combo{
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
		ncombs := make([]*Combo, 0, len(combos))
		for _, cmb := range combos {
			start := cmb.procs[len(cmb.procs)-1].loc
			for _, loc := range start.network {
				for _, cand := range loc.Allowed(proc, cmb.procs...) {
					nc := &Combo{
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

// instantRunoff selects the Combo with the most votes using instant runoff,
// with random tiebreaking.
func instantRunoff(combos []*Combo, scores map[*Location]map[*Combo]int32) *Combo {
	for {
		votes := make(map[*Combo]int)
		for loc, locScores := range scores {
			// Sort by the location's score and vote for highest score.
			slices.SortFunc(combos, func(a, b *Combo) int {
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
		slices.SortFunc(combos, func(a, b *Combo) int {
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

// Place emplaces the highest-voted production web to the
// locations, returning the placed combination. Each location
// ranks the combinations by score, then an instant-runoff
// election is held.
func Place(webs []*cpb.Web, locs []*Location) *Combo {
	combos := make([]*Combo, 0, len(locs))
	for _, web := range webs {
		if curr := generate(web, locs); len(curr) > 0 {
			combos = append(combos, curr...)
		}
	}

	if len(combos) == 0 {
		return nil
	}

	scores := make(map[*Location]map[*Combo]int32)
	for _, cmb := range combos {
		for _, proc := range cmb.procs {
			loc := proc.loc
			if scores[loc] == nil {
				scores[loc] = make(map[*Combo]int32)
			}
			scores[loc][cmb] = loc.score(proc)
		}
	}

	return instantRunoff(combos, scores)
}
