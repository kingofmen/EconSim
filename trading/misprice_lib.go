// Package misprice contains functions to search for mispricings.
package misprice

import (
	"fmt"
	"sort"
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

func (p price) String() string {
	if p.size == 0 {
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
	if p.callAsk.size < 1 {
		return 0
	}
	if p.putBid.size < 1 {
		return 0
	}
	return p.putBid.money - p.callAsk.money
}

func createPairs(quotes map[*polygon.Ticker]*marketdata.OptionQuote) []*pair {
	pairMap := make(map[string]*pair)
	for tck, quo := range quotes {
		exp, err := time.Parse("2006-01-02", tck.ExpDate)
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
	ret := fmt.Sprintf("Date %s strike %v\n  sell put %v\n  buy call %v", s.long.expire, s.long.strike, s.long.putBid.money, s.long.callAsk.money)
	if s.short != nil {
		ret = fmt.Sprintf("%s\n  buy put %v at %v", ret, s.short.strike, s.short.putAsk.money)
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
			if p2.expire != p.expire {
				continue
			}
			if lp-p2.putAsk.money < minPremium {
				continue
			}
			cand.short = p2
			break
		}
	}
	return found
}

func Search(quotes map[*polygon.Ticker]*marketdata.OptionQuote) {
	fmt.Printf("Received %d quotes.\n", len(quotes))
	pairs := createPairs(quotes)
	fmt.Printf("Created %d pairs.\n", len(pairs))
	suggest := search(pairs)
	fmt.Printf("Found %d suggestions.\n", len(suggest))
	for _, s := range suggest {
		if s.short != nil {
			fmt.Printf("%s\n", s)
		}
	}
}
