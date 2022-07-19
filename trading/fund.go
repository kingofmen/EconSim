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
				"BKR": 7153957,
				"TS": 5340353,
				"NOV": 8515430,
				"HP": 3025141,
				"CHX": 6422316,
				"FTI": 19536477,
				"PTEN": 8249831,
				"WHD": 2837181,
				"RIG": 33874052,
				"LBRT": 6612919,
				"NEX": 6947109,
				"OII": 4936594,
				"RES": 7661550,
				"NBR": 448970,
				"SLCA": 3750386,
				"DRQ": 1585264,
				"XPRO": 3877588,
				"PUMP": 4159378,
				"WTTR": 5465953,
				"CLB": 1917481,
				"HLX": 7877913,
				"OIS": 2935513,
				"BOOM": 557791,
			},
			outstanding: 11450000, // As of 7/13/22.
			cash: 2822236,
		},
	}
)

type fund struct {
	ticker string
	shares map[string]int32
	outstanding float64
	cash float64
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

func bpToDollars(bp int32) float64 {
	val := float64(bp)
	return val*0.001
}

// Analyse does the arbitrage analysis.
func Analyse(ticker, priceDate string, api tools.FinanceAPI) error {
	if !Exists(ticker) {
		return fmt.Errorf("Unknown fund %q", ticker)
	}
	fund := fundMap[ticker]
	if len(fund.shares) == 0 {
		return fmt.Errorf("%s has no shares")
	}
	fund.pricesBP = make(map[string]int32)
	tickers := []string{ticker}
	totalPrice := fund.cash
	for t := range fund.shares {
		tickers = append(tickers, t)
	}
	if len(priceDate) == 0 {
		priceDate = lookupDate()
	}
	for _, t := range tickers {
		
		p, err := api.LookupStock(t, priceDate)
		if err != nil {
			return fmt.Errorf("Error looking up %s price: %v", t, err)
		}
		fund.pricesBP[t] = p.CloseBP
		fmt.Printf("%s close price: %.2f\n", t, bpToDollars(p.CloseBP))
		totalPrice += bpToDollars(p.CloseBP) * float64(fund.shares[t])
	}
	totalPrice /= fund.outstanding
	fmt.Printf("Predicted share price: %.2f\n", totalPrice)
	diff := totalPrice - bpToDollars(fund.pricesBP[ticker])
	fmt.Printf("Difference: %.2f (%.2f%%)\n", diff, 100*diff/totalPrice)
	return nil
}
