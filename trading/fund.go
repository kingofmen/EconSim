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

// fund contains data about an ETF.
type fund struct {
	ticker string
	shares map[string]int32
	outstanding float64
	cash float64
	prices map[string]tools.BP
}

// dateString returns a year-month-day string.
func dateString(t time.Time, dashes bool) string {
	year, month, day := t.Date()
	sep := ""
	if dashes {
		sep = "-"
	}
	return fmt.Sprintf("%d%s%02d%s%d", year, sep, int(month), sep, day)
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
	return dateString(now, true)
}

// Exists returns true if the ticker is known.
func Exists(ticker string) bool {
	_, exists := fundMap[ticker]
	return exists
}

// guessNearestOptions returns estimates of the closest option strike prices
// below and above the provided stock price.
func guessNearestOptions(price tools.BP) (int32, int32) {
	cents := price / 10
	cents = cents % 100
	dollars := int32(price / 10 - cents)
	dollars = dollars / 100

	if dollars < 200 {
		return dollars, dollars+1
	}
	dollars -= (dollars % 5)
	return dollars, dollars+5
}

// fridayThreeMonthsAhead returns a date string for a Friday roughly three months
// from today's date.
func fridayThreeMonthsAhead() string {
	now := time.Now().AddDate(0, 3, 0)
	for now.Weekday() != time.Friday {
		now = now.AddDate(0, 0, 1)
	}
	return dateString(now, false)
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
	fund.prices = make(map[string]tools.BP)
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
		fund.prices[t] = p.Close
		fmt.Printf("%s close price: %s\n", t, p.Close)
		totalPrice += tools.BPToDollars(p.Close) * float64(fund.shares[t])
	}
	totalPrice /= fund.outstanding
	fmt.Printf("Predicted share price: %.2f\n", totalPrice)
	diff := totalPrice - tools.BPToDollars(fund.prices[ticker])
	fmt.Printf("Difference: %.2f (%.2f%%)\n", diff, 100*diff/totalPrice)

	for _, t := range tickers {
		_, up := guessNearestOptions(fund.prices[t])
		opt := &tools.Option{
			Ticker: t,
			EndDate: fridayThreeMonthsAhead(),
			PriceDollars: up,
			Call: true,
		}
		p, err := api.LookupOption(opt, priceDate)
		if err != nil {
			return fmt.Errorf("Error looking up option %v: %v", opt, err)
		}
		fmt.Printf("Price of call option on %s: %s", t, p.Close)
	}
	
	return nil
}
