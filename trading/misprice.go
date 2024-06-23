// Package misprice provides a binary to search for exploitable mispricings.
package main

import (
	"flag"
	"fmt"
	"log"
	"os"

	"trading/marketdata"
	"trading/misprice"
	"trading/polygon"
)

var (
	// NB marketdata will allow AAPL lookups with no API token.
	tickerF    = flag.String("ticker", "AAPL", "Underlying ticker symbol.")
	priceMin   = flag.Float64("price_min", -1.0, "Minimum strike price.")
	priceMax   = flag.Float64("price_max", -1.0, "Maximum strike price.")
	loadCache  = flag.Bool("load_cache", false, "If set, read existing API responses from file.")
	writeCache = flag.Bool("write_cache", false, "If set, save new API results to cache file.")
)

func main() {
	flag.Parse()

	if *loadCache {
		if err := polygon.LoadCache(); err != nil {
			log.Fatalf("Could not read cache: %v", err)
		}
	}

	params := &polygon.ListParams{
		Ticker:    *tickerF,
		MinStrike: *priceMin,
		MaxStrike: *priceMax,
		Limit:     1,
	}
	poly := polygon.Client{}
	options, err := poly.ListOptions(params)
	if err != nil {
		log.Fatalf("Could not look up option contracts: %v", err)
	}

	quotes := make(map[*polygon.Ticker]*marketdata.OptionQuote)
	for _, opt := range options {
		/*
			    // Polygon API - needs full account privileges which are Expensive.
					quote, err := poly.GetOptionQuotes(opt.Ticker)
					if err != nil {
						log.Fatalf("Could not get quotes for %s: %v", opt.Ticker, err)
					}
					fmt.Printf("%s: %+v", opt.Ticker, quote)
		*/

		quote, err := marketdata.GetOptionQuote(opt.Ticker)
		if err != nil {
			log.Fatalf("Could not get quote for %s: %v", opt.Ticker, err)
		}
		quotes[opt] = quote
		fmt.Printf("%s: %+v\n", opt.Ticker, quote)
	}

	misprice.Search(quotes)

	if *writeCache {
		if err := polygon.SaveCache(); err != nil {
			log.Fatalf("Could not read cache: %v", err)
		}
	}
	os.Exit(0)
}
