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
	endDateF = flag.String("enddate", "", "Ending date of options, in the format '220115' (YYMMDD).")
	priceDateF = flag.String("pricedate", "", "Date to look up prices, in the format '2022-01-15'.")
	strikeF = flag.Int("strike", 200, "Strike price in dollars.")
	loadCache = flag.Bool("load_cache", true, "If set, read existing API responses from file.")
	writeCache = flag.Bool("write_cache", true, "If set, save new API results to cache file.")
	lookupF = flag.Bool("lookup", false, "If set, skip fund analysis.")
	findChain = flag.Bool("chain", false, "If set, search for an option chain.")
)

func main() {
	flag.Parse()

	if *loadCache {
		if err := polygon.LoadCache(); err != nil {
			log.Fatalf("Could not read cache: %v", err)
		}
	}
	if !*lookupF && fund.Exists(*tickerF) {
		if *findChain {
			if err := fund.FindChains(*tickerF, &polygon.PolygonAPI{}); err != nil {
				log.Fatalf("Error finding chains: %v", err)
			}
			os.Exit(0)
		}
		if err := fund.Analyse(*tickerF, *priceDateF, *endDateF, &polygon.PolygonAPI{}); err != nil {
			log.Fatalf("Error analysing fund: %v", err)
		}
		if *writeCache {
			if err := polygon.SaveCache(); err != nil {
				log.Fatalf("Could not read cache: %v", err)
			}
		}
		os.Exit(0)
	}

	opt := &tools.Option{
		Ticker: *tickerF,
		EndDate: *endDateF,
		PriceDollars: int32(*strikeF),
		Call: true,
	}
	if err := polygon.Lookup(opt, *priceDateF); err != nil {
		log.Fatalf("Error in lookup: %v", err)
	}
}
