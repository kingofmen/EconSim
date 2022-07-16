// Package main provides an app to look up options data.
package main

import (
	"flag"
	"log"
	"os"

	"trading/fund"
	"trading/polygon"
)

var (
	tickerF = flag.String("ticker", "TSLA", "Ticker symbol.")
	endDateF = flag.String("enddate", "221021", "Ending date of options.")
	priceDateF = flag.String("pricedate", "", "Date to look up prices.")
	strikeF = flag.Int("strike", 200, "Strike price in dollars.")
)

func main() {
	flag.Parse()

	if fund.Exists(*tickerF) {
		if err := fund.Analyse(*tickerF); err != nil {
			log.Fatalf("Error analysing fund: %v", err)
		}
		os.Exit(0)
	}

	if err := polygon.Lookup(*tickerF, *endDateF, *priceDateF, *strikeF); err != nil {
		log.Fatalf("Error in lookup: %v", err)
	}
}
