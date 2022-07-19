// Package polygon contains methods for interacting with the Polygon.io API.
package polygon

import (
	"encoding/json"
	"fmt"
	"io"
	"math"
	"net/http"
	"time"

	"trading/tools"
)

const (
	apiToken = "lEKU4KMzBOJWda2UaVN7TeOsskQSMBs2"
	apiURL = "https://api.polygon.io/v2/aggs/ticker/%s/range/1/day/%s/%s"
)

var (
	timeStamp time.Time
	// Rate-limit is five API calls per minute.
	rateLimitSeconds = 12.0
	rateLimitPause = time.Second
)

// PolygonAPI is a struct for talking to the Polygon finance API. It satisfies FinanceAPI.
type PolygonAPI struct {

}

// polyAggregate is a model of the JSON returned by Polygon's 'aggs' endpoint.
type polyAggregate struct {
	Ticker string `json:"ticker"`
	Results []map[string]float64 `json:"results"`
}

// rateLimit implements a rate limitation.
func rateLimit() {
	if timeStamp.IsZero() {
		return
	}

	for {
		if d := time.Since(timeStamp); d.Seconds() > rateLimitSeconds {
			break
		}
		time.Sleep(rateLimitPause)
	}
}

func apiGet(ticker, date string) (*polyAggregate, error) {
	url := tickerURL(ticker, date)
	req, err := makeRequest(url)
	if err != nil {
		return nil, fmt.Errorf("Couldn't create request: %v", err)
	}

	rateLimit()
	timeStamp = time.Now()
	res, err := http.DefaultClient.Do(req)
	if err != nil {
		return nil, fmt.Errorf("Bad response: %v", err)
	}
	defer res.Body.Close()
	b, err := io.ReadAll(res.Body)
	if err != nil {
		return nil, fmt.Errorf("Couldn't read response body: %v", err)
	}
	agg := &polyAggregate{}
	if err := json.Unmarshal(b, agg); err != nil {
		return nil, fmt.Errorf("Couldn't unmarshal response: %v", err)
	}
	return agg, nil
}

func (p *PolygonAPI) LookupStock(ticker, date string) (*tools.TickerPrices, error) {
	agg, err := apiGet(ticker, date)
	if err != nil {
		return nil, err
	}
	if len(agg.Results) == 0 {
		return nil, fmt.Errorf("No results found for %s at %s", ticker, date)
	}
	value := agg.Results[0]
	return &tools.TickerPrices{
		Ticker: agg.Ticker,
		CloseBP: int32(math.Round(value["c"]*1000)),
		HighestBP: int32(math.Round(value["h"]*1000)),
		LowestBP: int32(math.Round(value["l"]*1000)),
		Transactions: int(math.Round(value["n"])),
	}, nil
}

func (p *PolygonAPI) LookupOption(opt *tools.Option) (*tools.TickerPrices, error) {
	return nil, nil
}

func tickerURL(ticker, date string) string {
	if len(date) == 0 {
		year, month, day := time.Now().Date()
		date = fmt.Sprintf("%d-%02d-%d", year, int(month), day)
	}
	return fmt.Sprintf(apiURL, ticker, date, date)
}

func makeRequest(url string) (*http.Request, error) {
	req, err := http.NewRequest(http.MethodGet, url, nil)
	if err != nil {
		return nil, err
	}
	req.Header.Set("Authorization", "Bearer " + apiToken)
	return req, nil
}

// Lookup prints information about the provided option.
func Lookup(opt *tools.Option, priceDate string) error {
	agg, err := apiGet(fmt.Sprintf("O:%s", opt.String()), priceDate)
	if err != nil {
		return err
	}
	fmt.Printf("Result: %v\n", agg)
	return nil
}
