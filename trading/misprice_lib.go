// Package misprice contains functions to search for mispricings.
package misprice

import (
	"fmt"
	"math"
	"sort"
	"strings"
	"time"

	"trading/marketdata"
	"trading/polygon"
)

const (
	// minPremium is the minimum required premium for the suggestion filter.
	minPremium = 0.01
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
		buyStr = fmt.Sprintf("Buy:\n  %s", strings.Join(acts, "  \n"))
	}
	acts = make([]string, 0, len(s.sell))
	for _, s := range s.sell {
		acts = append(acts, s.String())
	}
	if len(acts) > 0 {
		sellStr = fmt.Sprintf("Sell:\n  %s", strings.Join(acts, "  \n"))
	}

	return fmt.Sprintf("%s\n%s", buyStr, sellStr)
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

// profit returns the profit per option at the given
// expiry price.
func (s *suggestion) profit(final float64) float64 {
	ret := 0.0
	if s == nil {
		return ret
	}
	if s.long != nil {
		ret += s.long.putBid.money
		ret -= s.long.callAsk.money
		// Either put or call will be exercised;
		// pay strike, receive stock.
		ret -= s.long.strike
	}
	if s.short != nil {
		ret -= s.short.putAsk.money
		// Exercise put if below strike, otherwise
		// sell at market.
		if final < s.short.strike {
			ret += s.short.strike
		} else {
			ret += final
		}
	} else {
		// Receive the stock.
		ret += final
	}
	return ret
}

// search looks through the pairs for those where selling the put
// more than pays for buying the call.
func search(pairs []*pair) []*suggestion {
	found := make([]*suggestion, 0, len(pairs))
	for _, p := range pairs {
		lp := p.longPremium()
		if lp <= minPremium {
			continue
		}
		cand := &suggestion{
			long: p,
		}
		found = append(found, cand)
		for _, p2 := range pairs {
			if p2.strike <= p.strike {
				continue
			}
			if p2.expire.Before(p.expire) {
				continue
			}
			if !p2.putAsk.valid() {
				continue
			}
			// Use break-even here as that prints if the price
			// goes over the high strike. Being 99% break-even and
			// 1% profit is valuable.
			if lp-p2.putAsk.money < 0 {
				continue
			}
			cand.short = p2
			break
		}
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
					kind:   "put",
				},
				&action{
					strike: cand.short.strike,
					expire: cand.short.expire,
					price:  cand.short.callAsk.money,
					kind:   "call",
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
					kind:   "put",
				},
			},
			sell: []*action{
				&action{
					strike: cand.long.strike,
					expire: cand.long.expire,
					price:  cand.long.putBid.money,
					kind:   "put",
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
	premiumArb(pairs)
	strats := timeArb(pairs)
	for _, st := range strats {
		fmt.Printf("%s\n", st)
	}
	suggest := search(pairs)
	fmt.Printf("Found %d candidates.\n", len(suggest))
	count := 0
	for _, s := range suggest {
		if s.short != nil {
			fmt.Printf("%s\n", s)
			count++
		}
	}
	if count == 0 {
		fmt.Printf("Backup print:\n")
		for _, s := range suggest {
			fmt.Printf("%s\n", s)
		}
	}
}
