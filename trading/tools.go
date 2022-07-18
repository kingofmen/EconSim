// Package tools defines interfaces and utility methods for interacting with prices.
package tools

import (
	"fmt"
)

// TickerPrices holds price information for a ticker.
type TickerPrices struct {
	Ticker string
	CloseBP int32
	HighestBP int32
	LowestBP int32
	Transactions int
}

// Option models an option contract. It satisfies Stringer.
type Option struct {
	Ticker string
	EndDate string
	PriceDollars int
	Call bool
}

// GetType returns 'C' for a call and 'P' for a put.
func (opt *Option) GetType() string {
	if opt.Call {
		return "C"
	}
	return "P"
}

// String returns a standard-format representation of the option, e.g. TSLA210903C00700000.
func (opt *Option) String() string {
	return fmt.Sprintf("%s%s%s%08d", opt.Ticker, opt.EndDate, opt.GetType(), opt.PriceDollars*1000)
}

// FinanceAPI defines an interface for talking to a finance API.
type FinanceAPI interface {
	LookupStock(ticker, date string) (*TickerPrices, error)
	LookupOption(opt *Option) (*TickerPrices, error)
}


