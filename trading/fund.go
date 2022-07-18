// Package fund contains methods for modelling ETFs.
package fund

import (
	"fmt"
	"time"

	"trading/tools"
)

var (
	fundMap = map[string]*fund{
		"OIH": &fund{
			ticker: "OIH",
			shares: map[string]int32{
				"SLB": 14608675,
				"HAL": 10610891,
			},
		},
	}
)

type fund struct {
	ticker string
	shares map[string]int32
	pricesBP map[string]int32
}

// tradingDay returns true if the time represents a weekday.
func tradingDay(t time.Time) bool {
	day := t.Weekday()
	return day != time.Sunday && day != time.Saturday
}

// lookupDate returns a string representation of the previous trading day.
func lookupDate() string {
	now := time.Now()
	for !tradingDay(now) {
		now = now.AddDate(0, 0, -1)
	}
	year, month, day := now.Date()
	return fmt.Sprintf("%d-%02d-%d", year, int(month), day)
}

// Exists returns true if the ticker is known.
func Exists(ticker string) bool {
	_, exists := fundMap[ticker]
	return exists
}

// Analyse does the arbitrage analysis.
// Placeholder.
func Analyse(ticker string, api tools.FinanceAPI) error {
	if !Exists(ticker) {
		return fmt.Errorf("Unknown fund %q", ticker)
	}
	fund := fundMap[ticker]
	if len(fund.shares) == 0 {
		return fmt.Errorf("%s has no shares")
	}
	fund.pricesBP = make(map[string]int32)
	tickers := []string{ticker}
	for t := range fund.shares {
		tickers = append(tickers, t)
	}
	for _, t := range tickers {
		p, err := api.LookupStock(t, lookupDate())
		if err != nil {
			return fmt.Errorf("Error looking up %s price: %v", t, err)
		}
		fund.pricesBP[t] = p.CloseBP
		fmt.Printf("%s close price: %d\n", t, p.CloseBP)
	}
	return nil
}
