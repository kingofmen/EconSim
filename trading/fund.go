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
			weights: map[string]float64{
				"SLB": 19.54,
			},
		},
	}
)

type fund struct {
	ticker string
	weights map[string]float64
	prices map[string]float64
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
	if len(fund.weights) == 0 {
		return fmt.Errorf("%s has no weights")
	}
	fund.prices = make(map[string]float64)
	fundPrice, err := api.LookupStock(ticker, lookupDate())
	if err != nil {
		return fmt.Errorf("Error looking up fund: %v", err)
	}
	fmt.Printf("Fund price: %v\n", fundPrice)
	return nil
}
