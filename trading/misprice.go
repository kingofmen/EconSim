// Package misprice provides a binary to search for exploitable mispricings.
package main

import (
	"encoding/gob"
	"errors"
	"flag"
	"fmt"
	"io"
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
	limit      = flag.Int("limit", 100, "Number of options to list.")
	start      = flag.String("start", "", "Start date (inclusive) in YYYY-MM-DD format.")
	end        = flag.String("end", "", "End date (inclusive) in YYYY-MM-DD format.")
	cacheFile  = flag.String("cache", "C:\\Users\\rolfa\\base\\trading\\mispriceCache", "Location of the cache file.")
)

func analyseTicker(ticker string) error {
	params := &polygon.ListParams{
		Ticker:    ticker,
		MinStrike: *priceMin,
		MaxStrike: *priceMax,
		Limit:     *limit,
		StartDate: *start,
		EndDate:   *end,
	}
	poly := polygon.Client{}
	options, err := poly.ListOptions(params)
	if err != nil {
		return fmt.Errorf("Could not look up option contracts: %w", err)
	}

	quotes := make(map[*polygon.Ticker]*marketdata.OptionQuote)
	cached := make(map[string]*marketdata.OptionQuote)
	if *loadCache {
		if err := readCache(cached); err != nil {
			return fmt.Errorf("Could not read cache: %w", err)
		}
	}
	for _, opt := range options {
		var quote *marketdata.OptionQuote
		read := false
		if quote, read = cached[opt.Ticker]; !read {
			quote, err = marketdata.GetOptionQuote(opt.Ticker)
			if err != nil {
				return fmt.Errorf("Could not get quote for %s: %w", opt.Ticker, err)
			}
			quotes[opt] = quote
		}
		fmt.Printf("%s: %+v (%v) \n", opt.Ticker, quote, read)
		cached[opt.Ticker] = quote
	}
	if *writeCache {
		if err := saveCache(cached); err != nil {
			return fmt.Errorf("Could not write cache: %w", err)
		}
	}

	misprice.Search(quotes)
	return nil
}

func listTickers() {
	poly := polygon.Client{}
	params := &polygon.ListParams{
		Limit: *limit,
	}
	tickers, err := poly.ListTickers(params)
	if err != nil {
		log.Fatalf("Error listing tickers: %v", err)
	}
	for _, t := range tickers {
		fmt.Printf("%s\n", t)
	}
}

// readCache reads a cache map from file.
func readCache(quotes map[string]*marketdata.OptionQuote) error {
	f, err := os.Open(*cacheFile)
	if err != nil {
		if errors.Is(err, os.ErrNotExist) {
			// Not a problem, just means nothing is cached.
			return nil
		}
		return err
	}
	defer f.Close()
	dec := gob.NewDecoder(f)
	if err := dec.Decode(&quotes); err != nil {
		if errors.Is(err, io.EOF) {
			// Expected!
			return nil
		}
		return err
	}
	return nil
}

// saveCache writes the result map to file.
func saveCache(quotes map[string]*marketdata.OptionQuote) error {
	f, err := os.Create(*cacheFile)
	if err != nil {
		return err
	}
	defer f.Close()
	cod := gob.NewEncoder(f)
	return cod.Encode(quotes)
}

func main() {
	flag.Parse()

	if *tickerF == "random" {
		listTickers()
	} else if err := analyseTicker(*tickerF); err != nil {
		log.Fatalf("Error: %v\n", err)
	}

	os.Exit(0)
}
