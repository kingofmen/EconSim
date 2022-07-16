// Package fund contains methods for modelling ETFs.
package fund

import (
	"fmt"
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

// Exists returns true if the ticker is known.
func Exists(ticker string) bool {
	_, exists := fundMap[ticker]
	return exists
}

// Analyse does the arbitrage analysis.
// Placeholder.
func Analyse(ticker string) error {
	if !Exists(ticker) {
		return fmt.Errorf("Unknown fund %q", ticker)
	}
	return nil
}
