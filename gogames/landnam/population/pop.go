// Package pop defines population dynamics.
package pop

import (
	"errors"
	"fmt"

	poppb "gogames/landnam/population/pop_go_proto"
)

const (
	kNoTradeGood = "no_trade_good"
)

// Manager contains methods for POP dynamics.
type Manager struct {
	types map[string]*poppb.PopType
}

// NewManager returns a Manager.
func NewManager() *Manager {
	return &Manager{
		types: map[string]*poppb.PopType{},
	}
}

// check ensures that the Manager is initialised.
func (mgr *Manager) check(method string) error {
	if mgr == nil {
		return fmt.Errorf("%q called on nil Manager")
	}
	if mgr.types == nil {
		mgr.types = map[string]*poppb.PopType{}
	}
	return nil
}

// WithTypes adds the types to the map, returning a slice of errors.
func (mgr *Manager) WithTypes(types []*poppb.PopType) []error {
	if err := mgr.check("WithTypes"); err != nil {
		return []error{err}
	}
	errs := make([]error, 0, len(types))
	for idx, tp := range types {
		key := tp.GetKey()
		if len(key) < 1 {
			errs = append(errs, fmt.Errorf("PopType %d has zero-length key", idx))
			continue
		}
		if _, ex := mgr.types[key]; ex {
			errs = append(errs, fmt.Errorf("Duplicate key %q in PopType %d", key, idx))
			continue
		}
		mgr.types[key] = tp
	}
	return errs
}

// popsExist returns a slice containing an error for each
// POP whose type the manager does not know about.
func (mgr *Manager) popsExist(pops []*poppb.Pop) []error {
	errs := make([]error, 0, len(pops))
	for idx, pop := range pops {
		key := pop.GetKind()
		if _, ex := mgr.types[key]; !ex {
			errs = append(errs, fmt.Errorf("Pop %d has unknown key %q", idx, key))
			continue
		}
	}
	return errs
}

// Validate returns any errors in the Pops.
func (mgr *Manager) Validate(pops []*poppb.Pop) []error {
	if err := mgr.check("Validate"); err != nil {
		return []error{err}
	}
	errs := mgr.popsExist(pops)
	return errs
}

// getGood returns the good the POP specializes in, or kNoTradeGood,
// and the level.
func getGood(pop *poppb.Pop) (string, int32) {
	good := pop.GetSpecialize().GetGood()
	if len(good) == 0 {
		return kNoTradeGood, int32(1)
	}
	return good, pop.GetSpecialize().GetLevel()
}

// countTrades returns a map from goods to the number of levels
// the POP has traded with in that good.
func countTrades(pop *poppb.Pop, others []*poppb.Pop) map[string]int32 {
	counts := make(map[string]int32)
	good, level := getGood(pop)
	if good == kNoTradeGood {
		return counts
	}
	for _, ot := range others {
		ogood, olvl := getGood(ot)
		if ogood == good {
			continue
		}
		if nlvl := counts[ogood] + olvl; nlvl <= level {
			counts[ogood] = nlvl
		}
	}
	counts[kNoTradeGood] /= 2
	return counts
}

// gainsFromTrade calculates the additional production the POP
// gains from trading with others.
func gainsFromTrade(gainPerLevel int32, pop *poppb.Pop, others []*poppb.Pop) int32 {
	counts := countTrades(pop, others)
	total := -gainPerLevel * pop.GetSpecialize().GetLevel()
	for _, c := range counts {
		total += c * gainPerLevel * 2
	}
	return total
}

// Produce creates prods in accordance with the POP templates.
// TODO: Add modifiers.
func (mgr *Manager) Produce(pops []*poppb.Pop, trades map[*poppb.Pop][]*poppb.Pop) error {
	if err := mgr.check("Produce"); err != nil {
		return err
	}
	if errs := mgr.popsExist(pops); len(errs) > 0 {
		return errors.Join(errs...)
	}

	for _, pop := range pops {
		tmp := mgr.types[pop.GetKind()]
		production := tmp.GetProduce()
		production += gainsFromTrade(tmp.GetTradeGainPerLevel(), pop, trades[pop])
		if production > 0 {
			pop.Prods += production
		}
	}
	return nil
}

// minimumFood consumes enough prods to get the POP up to its minimum,
// if it's available, and returns the amount of prods remaining.
func minimumFood(tmp *poppb.PopType, pop *poppb.Pop) int32 {
	avail := pop.GetProds()
	req := tmp.GetMinConsume() - pop.GetConsume()
	if req <= 0 {
		return avail
	}
	if avail > req {
		avail = req
	}
	pop.Prods -= avail
	pop.Consume += avail
	return pop.GetProds()
}

// sizeModifier returns the additional cost of a level of
// consumption due to POP size.
func sizeModifier(tmp *poppb.PopType, pop *poppb.Pop) int32 {
	size := tmp.GetSizConsume()
	if size < 1 {
		return 0
	}
	total := pop.GetPeople().GetInfants()
	total += pop.GetPeople().GetChildren()
	total += pop.GetPeople().GetYouths()
	total += pop.GetPeople().GetAdults()
	total += pop.GetPeople().GetElders()
	total /= size
	return total * tmp.GetIncConsume()
}

// use consumes the amount of prods given by the want function
// (if they exist, otherwise the amount available), and stores
// them in the target field. It returns the amount consumed.
func use(count int, tmp *poppb.PopType, pop *poppb.Pop, want func() []int32, target *int32, mods ...func(*poppb.PopType, *poppb.Pop) int32) int32 {
	reqs := want()
	nr := len(reqs)
	if nr < 1 {
		return 0
	}
	req := reqs[nr-1]
	if count < nr {
		req = reqs[count]
	}
	for _, m := range mods {
		req += m(tmp, pop)
	}
	avail := pop.GetProds()
	if avail > req {
		avail = req
	}
	pop.Prods -= avail
	*target += avail
	return avail
}

// Consume iterates over the POPs and distributes their available
// production to their priorities.
func (mgr *Manager) Consume(pops []*poppb.Pop) error {
	if err := mgr.check("Consume"); err != nil {
		return err
	}
	if errs := mgr.popsExist(pops); len(errs) > 0 {
		return errors.Join(errs...)
	}

	for _, pop := range pops {
		tmp := mgr.types[pop.GetKind()]
		if minimumFood(tmp, pop) < 1 {
			continue
		}

		count := 0
		for pop.GetProds() > 0 {
			used := use(count, tmp, pop, tmp.GetConsume, &(pop.Consume), sizeModifier)
			used += use(count, tmp, pop, tmp.GetCapital, &(pop.Capital))
			used += use(count, tmp, pop, tmp.GetMilitia, &(pop.Militia))
			used += use(count, tmp, pop, tmp.GetMeaning, &(pop.Meaning))
			if used < 1 {
				break
			}
			count++
		}
	}
	return nil
}

// Demographics does births and deaths.
// TODO: Modifiers.
func (mgr *Manager) Demographics(pops []*poppb.Pop) error {
	if err := mgr.check("Demographics"); err != nil {
		return err
	}
	if errs := mgr.popsExist(pops); len(errs) > 0 {
		return errors.Join(errs...)
	}
	for _, pop := range pops {
		tmp := mgr.types[pop.GetKind()]
		demo := pop.GetPeople()
		change := &poppb.Demographics{}
		// Aging.
		change.Infants = -demo.GetInfants() / 5
		change.Children = demo.GetInfants()/5 - demo.GetChildren()/5
		change.Youths = demo.GetChildren()/5 - demo.GetYouths()/5
		change.Adults = demo.GetYouths()/5 - demo.GetAdults()/40
		change.Elders = demo.GetAdults() / 40
		// Births.
		change.Infants += demo.GetYouths() * tmp.GetBirthsPerThousand().GetYouths() / 1000
		change.Infants += demo.GetAdults() * tmp.GetBirthsPerThousand().GetAdults() / 1000
		change.Infants += demo.GetElders() * tmp.GetBirthsPerThousand().GetElders() / 1000
		// Deaths.
		change.Infants -= demo.GetInfants() * tmp.GetDeathsPerThousand().GetInfants() / 1000
		change.Children -= demo.GetChildren() * tmp.GetDeathsPerThousand().GetChildren() / 1000
		change.Youths -= demo.GetYouths() * tmp.GetDeathsPerThousand().GetYouths() / 1000
		change.Adults -= demo.GetAdults() * tmp.GetDeathsPerThousand().GetAdults() / 1000
		change.Elders -= demo.GetElders() * tmp.GetDeathsPerThousand().GetElders() / 1000
		// Apply changes.
		demo.Infants += change.GetInfants()
		demo.Children += change.GetChildren()
		demo.Youths += change.GetYouths()
		demo.Adults += change.GetAdults()
		demo.Elders += change.GetElders()
		// Lower bound.
		if demo.Infants < 0 {
			demo.Infants = 0
		}
		if demo.Children < 0 {
			demo.Children = 0
		}
		if demo.Youths < 0 {
			demo.Youths = 0
		}
		if demo.Adults < 0 {
			demo.Adults = 0
		}
		if demo.Elders < 0 {
			demo.Elders = 0
		}
	}
	return nil
}
