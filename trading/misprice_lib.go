// Package misprice contains functions to search for mispricings.
package misprice

import (
	"fmt"
	"math"
	"slices"
	"sort"
	"strings"
	"time"

	"trading/marketdata"
	"trading/polygon"
)

const (
	// minPremium is the minimum required premium for the suggestion filter.
	minPremium = 0.01
	// callStr denotes a call.
	callStr = "call"
	// putStr denotes a put.
	putStr = "put"
	// minBoxSpread is the minimum difference in strike prices to consider a box spread.
	minBoxSpread = 5.1
	// maxBorrowRate is the maximum rate at which we'll borrow.
	maxBorrowRate = 0.05
	// minLendRate is the minimum rate of return for a loan to be of interest.
	minLendRate = 0.03
)

type price struct {
	money float64
	size  float64
}

func (p price) valid() bool {
	return p.size > 0.99
}

func (p price) String() string {
	if !p.valid() {
		return "N/A"
	}
	return fmt.Sprintf("%v @ %v", p.size, p.money)
}

type pair struct {
	strike  float64
	expire  time.Time
	putAsk  price
	putBid  price
	callAsk price
	callBid price
}

type action struct {
	ticker string
	strike float64
	expire time.Time
	price  float64
	kind   string
}

func (a *action) String() string {
	if a == nil {
		return "<nil>"
	}
	return fmt.Sprintf("%s %s %.2f %s @ %.2f", a.kind, a.ticker, a.strike, a.expire.Format(time.DateOnly), a.price)
}

func (p *pair) String() string {
	if p == nil {
		return "<nil>"
	}
	return fmt.Sprintf("{Strike %v on %s, put %s / %s, call %s / %s}", p.strike, p.expire.Format(time.DateOnly), p.putBid, p.putAsk, p.callBid, p.callAsk)
}

// boxSpreads constructs box spreads, with no filtering.
func boxSpreads(pairs []*pair) []*strategy {
	cands := pairFilter(pairs, []func(*pair, *pair) bool{
		sameDate,
		distinct,
		func(one, two *pair) bool {
			if !two.callAsk.valid() {
				return false
			}
			if !one.putBid.valid() {
				return false
			}
			if !two.callBid.valid() {
				return false
			}
			if !two.putAsk.valid() {
				return false
			}
			if strikeDiff := two.strike - one.strike; strikeDiff <= minBoxSpread {
				return false
			}
			return true
		}})

	strats := make([]*strategy, 0, len(cands))
	for _, cand := range cands {
		buyCall := &action{
			strike: cand.long.strike,
			expire: cand.long.expire,
			price:  cand.long.callAsk.money,
			kind:   callStr,
		}
		sellCall := &action{
			strike: cand.short.strike,
			expire: cand.short.expire,
			price:  cand.short.callBid.money,
			kind:   callStr,
		}
		buyPut := &action{
			strike: cand.short.strike,
			expire: cand.short.expire,
			price:  cand.short.putAsk.money,
			kind:   putStr,
		}
		sellPut := &action{
			strike: cand.long.strike,
			expire: cand.long.expire,
			price:  cand.long.putBid.money,
			kind:   putStr,
		}
		str := &strategy{
			buys: []*action{buyCall, buyPut},
			sell: []*action{sellCall, sellPut},
		}
		if str.profit(0) > 0 {
			strats = append(strats, str)
		}

		// Flip everything to make the short version.
		buyCall = &action{
			strike: cand.short.strike,
			expire: cand.short.expire,
			price:  cand.short.callAsk.money,
			kind:   callStr,
		}
		sellCall = &action{
			strike: cand.long.strike,
			expire: cand.long.expire,
			price:  cand.long.callBid.money,
			kind:   callStr,
		}
		buyPut = &action{
			strike: cand.long.strike,
			expire: cand.long.expire,
			price:  cand.long.putAsk.money,
			kind:   putStr,
		}
		sellPut = &action{
			strike: cand.short.strike,
			expire: cand.short.expire,
			price:  cand.short.putBid.money,
			kind:   putStr,
		}
		str = &strategy{
			buys: []*action{buyCall, buyPut},
			sell: []*action{sellCall, sellPut},
		}
		strats = append(strats, str)
	}
	return strats
}

// longPremium returns the revenue from buying the call and selling the put.
func (p *pair) longPremium() float64 {
	if p == nil {
		return 0
	}
	if !p.callAsk.valid() {
		return 0
	}
	if !p.putBid.valid() {
		return 0
	}
	return p.putBid.money - p.callAsk.money
}

func createPairs(quotes map[*polygon.Ticker]*marketdata.OptionQuote) []*pair {
	pairMap := make(map[string]*pair)
	for tck, quo := range quotes {
		exp, err := time.Parse(time.DateOnly, tck.ExpDate)
		if err != nil {
			fmt.Printf("Could not parse time string %q: %v", tck.ExpDate, err)
			continue
		}
		key := fmt.Sprintf("%s_%v", tck.ExpDate, tck.Price)
		p, ok := pairMap[key]
		if !ok {
			p = &pair{
				strike: tck.Price,
				expire: exp,
			}
			pairMap[key] = p
		}
		if tck.IsCall() {
			p.callAsk = price{quo.AskPrice[0], quo.AskSize[0]}
			p.callBid = price{quo.BidPrice[0], quo.BidSize[0]}
		} else {
			p.putAsk = price{quo.AskPrice[0], quo.AskSize[0]}
			p.putBid = price{quo.BidPrice[0], quo.BidSize[0]}
		}
	}
	pairs := make([]*pair, 0, len(pairMap))
	for _, p := range pairMap {
		pairs = append(pairs, p)
	}
	sort.Slice(pairs, func(i, j int) bool {
		one, two := pairs[i], pairs[j]
		if one.expire.Before(two.expire) {
			return true
		}
		if two.expire.Before(one.expire) {
			return false
		}
		if one.strike < two.strike {
			return true
		}
		if two.strike < one.strike {
			return false
		}
		if one.putAsk.money < two.putAsk.money {
			return true
		}
		if one.callAsk.money < two.callAsk.money {
			return true
		}
		return false
	})
	return pairs
}

type strategy struct {
	buys []*action
	sell []*action
}

func (s *strategy) String() string {
	if s == nil {
		return "<nil>"
	}
	buyStr := "Buy: Nothing\n"
	sellStr := "Sell: Nothing"

	acts := make([]string, 0, len(s.buys))
	for _, b := range s.buys {
		acts = append(acts, b.String())
	}
	if len(acts) > 0 {
		buyStr = fmt.Sprintf("Buy:\n  %s", strings.Join(acts, "\n  "))
	}
	acts = make([]string, 0, len(s.sell))
	for _, s := range s.sell {
		acts = append(acts, s.String())
	}
	if len(acts) > 0 {
		sellStr = fmt.Sprintf("Sell:\n  %s", strings.Join(acts, "\n  "))
	}

	return fmt.Sprintf("%s\n%s\n", buyStr, sellStr)
}

type suggestion struct {
	long  *pair
	short *pair
}

func (s *suggestion) String() string {
	if s == nil {
		return "<nil>"
	}
	if s.long == nil {
		return "no long"
	}
	ret := fmt.Sprintf("Date %s strike %v\n  sell put %v\n  buy call %v", s.long.expire.Format(time.DateOnly), s.long.strike, s.long.putBid.money, s.long.callAsk.money)
	if s.short != nil {
		dstr := ""
		if s.short.expire.After(s.long.expire) {
			dstr = fmt.Sprintf(" (%s)", s.short.expire.Format(time.DateOnly))
		}
		ret = fmt.Sprintf("%s\n  buy put %v at %v%s", ret, s.short.strike, s.short.putAsk.money, dstr)
	}
	return ret
}

// price returns the cost of the options, which may be negative.
func (s *strategy) price() float64 {
	ret := 0.0
	if s == nil {
		return ret
	}
	for _, buy := range s.buys {
		ret += buy.price
	}
	for _, sell := range s.sell {
		ret -= sell.price
	}
	return ret
}

// expiry returns the first expiry date among the options.
func (s *strategy) expiry() time.Time {
	var bkup time.Time
	if s == nil {
		return bkup
	}
	for _, buy := range s.buys {
		if buy.expire.IsZero() {
			continue
		}
		return buy.expire
	}
	for _, sell := range s.sell {
		if sell.expire.IsZero() {
			continue
		}
		return sell.expire
	}
	return bkup
}

// periods returns the fraction of a year until the option expires.
func (s *strategy) periods() float64 {
	duration := time.Until(s.expiry()).Truncate(24 * time.Hour)
	return float64(duration) / float64(365*24*time.Hour)
}

// yield returns the percentage return of the strategy, normalised
// to a year by linear approximation.
func (s *strategy) yield(final float64) float64 {
	if s == nil {
		return 0.0
	}
	percent := s.profit(final) / s.price()
	return percent / s.periods()
}

// value returns the expiry value of the options at the given price.
func (s *strategy) value(final float64) float64 {
	ret := 0.0
	if s == nil {
		return ret
	}
	for _, buy := range s.buys {
		if buy.kind == putStr {
			if v := buy.strike - final; v > 0 {
				ret += v
			}
		}
		if buy.kind == callStr {
			if v := final - buy.strike; v > 0 {
				ret += v
			}
		}
	}
	for _, sell := range s.sell {
		if sell.kind == putStr {
			if v := sell.strike - final; v > 0 {
				ret -= v
			}
		}
		if sell.kind == callStr {
			if v := final - sell.strike; v > 0 {
				ret -= v
			}
		}
	}
	return ret
}

// profit returns the profit per option at the given
// expiry price.
func (s *strategy) profit(final float64) float64 {
	if s == nil {
		return 0.0
	}
	return s.value(final) - s.price()
}

// spreadArb looks through the pairs for combinations such that
// buying a call and selling a put leaves enough money to buy
// a put at a higher strike, covering the difference.
func spreadArb(pairs []*pair) []*strategy {
	cands := pairFilter(pairs, []func(*pair, *pair) bool{
		distinct,
		func(one, two *pair) bool {
			lp := one.longPremium()
			if lp <= minPremium {
				return false
			}
			if two.strike <= one.strike {
				return false
			}
			if two.expire.Before(one.expire) {
				return false
			}
			if !two.putAsk.valid() {
				return false
			}
			// Use break-even here as that prints if the price
			// goes over the high strike. Being 99% break-even and
			// 1% profit is valuable.
			if lp-two.putAsk.money < 0 {
				return false
			}
			return true
		},
	})

	found := make([]*strategy, 0, len(cands))
	for _, cand := range cands {
		found = append(found, &strategy{
			buys: []*action{
				&action{
					kind:   callStr,
					strike: cand.long.strike,
					expire: cand.long.expire,
					price:  cand.long.callAsk.money,
				},
				&action{
					kind:   putStr,
					strike: cand.short.strike,
					expire: cand.short.expire,
					price:  cand.short.putAsk.money,
				},
			},
			sell: []*action{
				&action{
					kind:   putStr,
					strike: cand.long.strike,
					expire: cand.long.expire,
					price:  cand.long.putBid.money,
				},
			},
		})
	}
	return found
}

// premiumArb looks for put-call pairs such that the premium
// is less than the strike difference.
func premiumArb(pairs []*pair) []*strategy {
	cands := pairFilter(pairs, []func(*pair, *pair) bool{
		func(one, two *pair) bool {
			if !one.putAsk.valid() {
				return false
			}
			if !two.callAsk.valid() {
				return false
			}

			premium := one.putAsk.money + two.callAsk.money
			strikeDiff := one.strike - two.strike
			return premium <= strikeDiff
		},
	})
	strats := make([]*strategy, 0, len(cands))
	for _, cand := range cands {
		strats = append(strats, &strategy{
			buys: []*action{
				&action{
					strike: cand.long.strike,
					expire: cand.long.expire,
					price:  cand.long.putAsk.money,
					kind:   putStr,
				},
				&action{
					strike: cand.short.strike,
					expire: cand.short.expire,
					price:  cand.short.callAsk.money,
					kind:   callStr,
				},
			},
		})
	}

	return strats
}

// pairFilter finds pairs of pairs such that every filter returns
// true for that pair.
func pairFilter(pairs []*pair, filters []func(*pair, *pair) bool) []*suggestion {
	results := make([]*suggestion, 0, len(pairs))
	for _, p1 := range pairs {
		for _, p2 := range pairs {
			good := true
			for _, f := range filters {
				if !f(p1, p2) {
					good = false
					break
				}
			}
			if !good {
				continue
			}
			results = append(results, &suggestion{p1, p2})
		}
	}
	return results
}

// equalStrike returns true if the pairs have equal strike prices.
func equalStrike(one, two *pair) bool {
	return math.Abs(one.strike-two.strike) < 0.01
}

// after returns true if two is after one.
func after(one, two *pair) bool {
	return two.expire.After(one.expire)
}

// sameDate returns true if the expiry dates are the same.
func sameDate(one, two *pair) bool {
	return two.expire.Year() == one.expire.Year() && two.expire.YearDay() == one.expire.YearDay()
}

// distinct returns true if the pairs are not the same.
func distinct(one, two *pair) bool {
	return one != two
}

// timeArb looks for pairs of puts with equal strikes,
// different expiration dates, and same premium.
func timeArb(pairs []*pair) []*strategy {
	cands := pairFilter(pairs, []func(*pair, *pair) bool{
		equalStrike,
		after,
		distinct,
		func(one, two *pair) bool {
			if !one.putBid.valid() {
				return false
			}
			if !two.putAsk.valid() {
				return false
			}
			if one.putBid.money < two.putAsk.money {
				return false
			}
			return true
		},
	})
	results := make([]*strategy, 0, len(cands))
	for _, cand := range cands {
		results = append(results, &strategy{
			buys: []*action{
				&action{
					strike: cand.short.strike,
					expire: cand.short.expire,
					price:  cand.short.putAsk.money,
					kind:   putStr,
				},
			},
			sell: []*action{
				&action{
					strike: cand.long.strike,
					expire: cand.long.expire,
					price:  cand.long.putBid.money,
					kind:   putStr,
				},
			},
		})
	}
	return results
}

func Search(quotes map[*polygon.Ticker]*marketdata.OptionQuote) {
	fmt.Printf("Received %d quotes.\n", len(quotes))
	pairs := createPairs(quotes)
	fmt.Printf("Created %d pairs.\n", len(pairs))

	strats := premiumArb(pairs)
	strats = append(strats, spreadArb(pairs)...)
	strats = append(strats, timeArb(pairs)...)
	fmt.Printf("Found %d strategies.\n", len(strats))
	for _, st := range strats {
		fmt.Printf("%s\n", st)
	}

	spreads := boxSpreads(pairs)
	slices.SortFunc(spreads, func(one, two *strategy) int {
		oney, twoy := one.yield(0), two.yield(0)
		if oney < twoy {
			return -1
		} else if oney > twoy {
			return 1
		}
		return 0
	})
	for _, sp := range spreads {
		dir := "Lend"
		val := sp.value(0)
		yield := sp.yield(0)
		if val < 0 {
			if yield > maxBorrowRate {
				continue
			}
			dir = "Borrow"
		} else if yield < minLendRate {
			continue
		}
		fmt.Printf("%s %.2f, return %.2f, profit %.2f for yield %.2f%%:\n%s\n", dir, math.Abs(sp.price()), math.Abs(val), sp.profit(0), yield*100, sp)
	}
}
