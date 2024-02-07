// Package production models productive economic activity.
package production

import (
	prodpb "gogames/settlers/economy/production_proto"
)

const (
	tolerance = 0.001
)

// Producer is an interface for a production process.
type Producer interface {
	Makes() string
	Output() float64
	Run(map[string]float64) float64
}

// Alloc stores a priority allocation.
type Alloc struct {
	consume  float64
	meaning  float64
	invest   float64
	military float64
	invTotal float64
}

func EqualAlloc(init float64) *Alloc {
	a := &Alloc{
		consume:  init,
		meaning:  init,
		invest:   init,
		military: init,
	}
	a.Norm()
	return a
}

func NewAlloc(c, m, i, w float64) *Alloc {
	a := &Alloc{
		consume:  c,
		meaning:  m,
		invest:   i,
		military: w,
	}
	a.Norm()
	return a
}

// add adds to one of the four areas without renormalising.
func (a *Alloc) add(area prodpb.Area, amt float64) {
	switch area {
	case prodpb.Area_A_CONSUME:
		a.consume += amt
	case prodpb.Area_A_MEANING:
		a.meaning += amt
	case prodpb.Area_A_INVEST:
		a.invest += amt
	case prodpb.Area_A_MILITARY:
		a.military += amt
	}
}

// Norm renormalises the alloc.
func (a *Alloc) Norm() {
	total := a.consume + a.meaning + a.invest + a.military
	if total < tolerance {
		return
	}
	a.invTotal = 1.0 / total
}

func CalcAlloc(exist map[string]float64, buckets map[string]*prodpb.Good) *Alloc {
	alloc := EqualAlloc(0)
	for key, good := range buckets {
		amt := exist[key]
		if amt < tolerance {
			continue
		}
		alloc.add(good.GetArea(), amt)
	}
	alloc.Norm()
	return alloc
}

func (a *Alloc) Priority(area prodpb.Area) float64 {
	switch area {
	case prodpb.Area_A_CONSUME:
		return a.consume * a.invTotal
	case prodpb.Area_A_MEANING:
		return a.meaning * a.invTotal
	case prodpb.Area_A_INVEST:
		return a.invest * a.invTotal
	case prodpb.Area_A_MILITARY:
		return a.military * a.invTotal
	}
	return 0
}

// meets returns true if exists matches the requirement.
func meets(key string, exist map[string]float64, buckets map[string]*prodpb.Good, req *prodpb.Requirement) bool {
	switch req.GetKind() {
	case prodpb.Requirement_RK_FULL:
		target, ok := buckets[req.GetKey()]
		if !ok {
			return false
		}
		return exist[req.GetKey()] >= target.GetMaximum()
	case prodpb.Requirement_RK_CAP:
		return exist[req.GetKey()]*req.GetAmount() > exist[key]
	case prodpb.Requirement_RK_MIN:
		return exist[req.GetKey()] >= req.GetAmount()
	case prodpb.Requirement_RK_MAX:
		return exist[req.GetKey()] <= req.GetAmount()
	}
	return false
}

// filterGoods returns the bucket keys whose requirements are met by exist.
func filterGoods(exist map[string]float64, buckets map[string]*prodpb.Good) map[string]bool {
	possible := make(map[string]bool)
bucketLoop:
	for key, good := range buckets {
		if exist[key] >= good.GetMaximum() {
			continue
		}
		for _, req := range good.GetPrereqs() {
			if !meets(key, exist, buckets, req) {
				continue bucketLoop
			}
		}
		possible[key] = true
	}
	return possible
}

// Produce walks the decision tree for the given producers and
// runs the resulting production functions.
// TODO: Return a slice of decisions made.
func Produce(labor float64, exist map[string]float64, buckets map[string]*prodpb.Good, proc []Producer, target *Alloc) {
	count := 0
	for labor > tolerance {
		count++
		if count > 1000 {
			return
		}
		available := filterGoods(exist, buckets)
		if len(available) == 0 {
			return
		}

		currAlloc := CalcAlloc(exist, buckets)
		best := -1
		score := -1000.0
		for idx, p := range proc {
			if !available[p.Makes()] {
				continue
			}
			amt := p.Output()
			area := buckets[p.Makes()].GetArea()
			if curr := (target.Priority(area) - currAlloc.Priority(area)) * amt; curr > score {
				best = idx
				score = curr
			}
		}
		if best < 0 {
			return
		}

		labor -= proc[best].Run(exist)
	}
}
