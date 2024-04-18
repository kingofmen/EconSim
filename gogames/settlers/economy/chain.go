// Package chain implements production networks.
package chain

import (
	"google.golang.org/protobuf/proto"

	cpb "gogames/settlers/economy/chain_proto"
)

// Work models a production process at a particular location.
type Work struct {
	// Work type.
	key string
	// Labour used.
	assigned *cpb.Workers
}

type Scorer interface {
	Score(exist, marginal *cpb.Goods) int32
}

// ScorerFunc returns a score for the given basket.
type ScorerFunc func(exist, marginal *cpb.Goods) int32

func (sf ScorerFunc) Score(exist, marginal *cpb.Goods) int32 {
	return sf(exist, marginal)
}

// Location represents an area containing 'boxes' that can be
// filled with work processes; it is small enough that all the
// boxen can communicate with negligible frictional costs.
type Location struct {
	// Filled boxes.
	boxen []*Work
	// Labour pool.
	pool *cpb.Workers
	// Connected locations.
	network []*Location
	// Prioritiser.
	evaluator Scorer
	// Stockpile.
	goods *cpb.Goods
}

// super returns true if p1 is a superset of p2.
func super(p1, p2 *cpb.Workers) bool {
	avail := p1.GetSkills()
	for skill, amount := range p2.GetSkills() {
		if avail[skill] < amount {
			return false
		}
	}
	return true
}

// Allowed returns true if a box can be filled with the given process.
func (loc *Location) Allowed(p *cpb.Process) bool {
	if !super(loc.pool, p.GetWorkers()) {
		return false
	}
	// TODO: Other constraints.
	return true
}

// output returns what this location will produce by adding the process.
func (loc *Location) output(p *cpb.Process) *cpb.Goods {
	// TODO: Account for scaling and whatnot.
	return proto.Clone(p.GetOutputs()).(*cpb.Goods)
}

// defaultScore returns a score prioritising all goods equally
// with no diminishing returns.
func defaultScore(exist, output *cpb.Goods) int32 {
	ret := int32(0)
	for _, amt := range output.GetGoods() {
		ret += amt
	}
	return ret
}

// score returns the priority of the process.
func (loc *Location) score(p *cpb.Process) int32 {
	output := loc.output(p)
	if loc.evaluator != nil {
		return loc.evaluator.Score(loc.goods, output)
	}
	return defaultScore(loc.goods, output)
}

// assignWork fills a box with the provided process.
func (loc *Location) assignWork(w *Work) {
	for skill, amount := range w.assigned.GetSkills() {
		loc.pool.GetSkills()[skill] -= amount
	}
	loc.boxen = append(loc.boxen, w)
}

// Place assigns labour to the best available Process.
// It returns the key of the process chosen.
func (loc *Location) Place(options []*cpb.Process) string {
	legal := make([]*cpb.Process, 0, len(options))
	for _, opt := range options {
		if loc.Allowed(opt) {
			legal = append(legal, opt)
		}
	}
	if len(legal) == 0 {
		return ""
	}

	best, pts := legal[0], loc.score(legal[0])
	for _, opt := range legal[1:] {
		if curr := loc.score(opt); curr > pts {
			best = opt
			pts = curr
		}
	}

	loc.assignWork(&Work{
		key:      best.GetKey(),
		assigned: proto.Clone(best.GetWorkers()).(*cpb.Workers),
	})
	return best.GetKey()
}
