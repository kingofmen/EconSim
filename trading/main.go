// Package main provides an app to look up options data.
package main

import (
	"flag"
	"log"
	"os"

	"trading/fund"
	"trading/polygon"
	"trading/tools"
)

var (
	tickerF = flag.String("ticker", "TSLA", "Ticker symbol.")
	endDateF = flag.String("enddate", "221021", "Ending date of options, in the format '220115'.")
	priceDateF = flag.String("pricedate", "", "Date to look up prices, in the format '2022-01-15'.")
	strikeF = flag.Int("strike", 200, "Strike price in dollars.")
)

func main() {
	flag.Parse()

	if fund.Exists(*tickerF) {
		if err := fund.Analyse(*tickerF, &polygon.PolygonAPI{}); err != nil {
			log.Fatalf("Error analysing fund: %v", err)
		}
		os.Exit(0)
	}

	opt := &tools.Option{
		Ticker: *tickerF,
		EndDate: *endDateF,
		PriceDollars: *strikeF,
		Call: true,
	}
	if err := polygon.Lookup(opt, *priceDateF); err != nil {
		log.Fatalf("Error in lookup: %v", err)
	}
}
