// Package production models productive economic activity.
package production

import (
	prodpb "gogames/settlers/economy/production_proto"
)

const (
	tolerance = 0.001
)

type Producer interface {
	Makes() string
	Output() float64
	Run(map[string]float64) float64
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
		for _, req := range good.GetPrereqs() {
			if !meets(key, exist, buckets, req) {
				continue bucketLoop
			}
		}
		possible[key] = true
	}
	return possible
}

func calcAlloc(exist map[string]float64, buckets map[string]*prodpb.Good) map[prodpb.Area]float64 {
	alloc := map[prodpb.Area]float64{
		prodpb.Area_A_CONSUME:  0.01,
		prodpb.Area_A_MEANING:  0.01,
		prodpb.Area_A_INVEST:   0.01,
		prodpb.Area_A_MILITARY: 0.01,
	}
	total := 0.04
	for key, good := range buckets {
		amt := exist[key]
		if amt < tolerance {
			continue
		}
		alloc[good.GetArea()] += amt
		total += amt
	}
	invTotal := 1.0 / total
	for key, val := range alloc {
		alloc[key] = val * invTotal
	}
	return alloc
}

// Produce walks the decision tree for the given producers and
// runs the resulting production functions.
// TODO: Return a slice of decisions made.
func Produce(labor float64, exist map[string]float64, buckets map[string]*prodpb.Good, proc []Producer, alloc map[prodpb.Area]float64) {
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

		currAlloc := calcAlloc(exist, buckets)
		best := -1
		score := -1000.0
		for idx, p := range proc {
			if !available[p.Makes()] {
				continue
			}
			amt := p.Output()
			area := buckets[p.Makes()].GetArea()
			if curr := (alloc[area] - currAlloc[area]) * amt; curr > score {
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
