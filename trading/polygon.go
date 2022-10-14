// Package polygon contains methods for interacting with the Polygon.io API.
package polygon

import (
	"encoding/gob"
	"encoding/json"
	"errors"
	"fmt"
	"io"
	"math"
	"net/http"
	"os"
	"time"

	"trading/tools"
)

const (
	apiToken = "lEKU4KMzBOJWda2UaVN7TeOsskQSMBs2"
	aggURL = "https://api.polygon.io/v2/aggs/ticker/%s/range/1/day/%s/%s"
	contractURL = "https://api.polygon.io/v3/reference/options/contracts"
	cacheFile = "C:\\Users\\Rolf\\base\\bazel-out\\tradingCache"
)

var (
	// Rate-limit is five API calls per minute.
	rateLimitPause = time.Minute

	// Map from request to response for caching.
	cacheMap = map[string]*polyAggregate{}

	// True if cache needs updating.
	cacheDirty = false
)

// PolygonAPI is a struct for talking to the Polygon finance API. It satisfies FinanceAPI.
type PolygonAPI struct {

}

// polyAggregate is a model of the JSON returned by Polygon's 'aggs' endpoint.
type polyAggregate struct {
	ticker string `json:"ticker"`
	results []map[string]float64 `json:"results"`
}

// contracts is a model of the JSON returned by Polygon's "options/contracts/ticker" endpoint.
// Note that the 'additional_underlyings' field is ignored.
type contract struct {
	cfi string `json:"cfi"`
	contractType string `json:"contract_type"`
	style string `json:"exercise_style"`
	expDate string `json:"expiration_date"`
	exchange string `json:"primary_exchange"`
	shares int `json:"shares_per_contract"`
	price string `json:"strike_price"`
	ticker string `json:"ticker"`
	underlying string `json:"underlying_ticker"`
}

// optionContracts is a model of the JSON returned by Polygon's "options/contracts" endpoint.
type optionContracts struct {
	requestID string `json:"request_id"`
	results []*contract `json:"results"`
}

func handleResponse(res *http.Response) (*polyAggregate, error) {
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

func getAgg(ticker, date string) (*polyAggregate, error) {
	url := tickerURL(ticker, date)
	if cached, ok := cacheMap[url]; ok {
		return cached, nil
	}
	req, err := makeRequest(url)
	if err != nil {
		return nil, fmt.Errorf("Couldn't create request: %v", err)
	}

	res, err := http.DefaultClient.Do(req)
	if res.StatusCode == 429 {
		// Rate limit.
		fmt.Printf("Received %q, pausing before retry\n", res.Status)
		time.Sleep(rateLimitPause)
		res, err = http.DefaultClient.Do(req)
	}
	if err != nil {
		return nil, fmt.Errorf("Bad response: %v", err)
	}
	agg, err := handleResponse(res)
	if err != nil {
		return nil, err
	}
	if len(agg.results) == 0 {
		return nil, fmt.Errorf("%w for %s at %s (%s)", tools.NoResults, ticker, date, url)
	}
	cacheMap[url] = agg
	cacheDirty = true
	return agg, nil
}

func convertAggregate(agg *polyAggregate) *tools.TickerPrices {
	value := agg.results[0]
	return &tools.TickerPrices{
		Ticker: agg.ticker,
		Close: tools.BP(math.Round(value["c"]*1000)),
		Highest: tools.BP(math.Round(value["h"]*1000)),
		Lowest: tools.BP(math.Round(value["l"]*1000)),
		Transactions: int(math.Round(value["n"])),
	}
}

func (p *PolygonAPI) LookupStock(ticker, date string) (*tools.TickerPrices, error) {
	agg, err := getAgg(ticker, date)
	if err != nil {
		return nil, err
	}
	return convertAggregate(agg), nil
}

func (p *PolygonAPI) LookupOption(opt *tools.Option, date string) (*tools.TickerPrices, error) {
	agg, err := getAgg(opt.String(), date)
	if err != nil {
		return nil, err
	}
	return convertAggregate(agg), nil
}

func tickerURL(ticker, date string) string {
	if len(date) == 0 {
		year, month, day := time.Now().Date()
		date = fmt.Sprintf("%d-%02d-%d", year, int(month), day)
	}
	return fmt.Sprintf(aggURL, ticker, date, date)
}

func makeRequest(url string) (*http.Request, error) {
	req, err := http.NewRequest(http.MethodGet, url, nil)
	if err != nil {
		return nil, err
	}
	req.Header.Set("Authorization", "Bearer " + apiToken)
	return req, nil
}

// LoadCache reads a cache map from file.
func LoadCache() error {
	f, err := os.Open(cacheFile)
	if err != nil {
		if errors.Is(err, os.ErrNotExist) {
			// Not a problem, just means nothing is cached.
			return nil
		}
		return err
	}
	defer f.Close()
	dec := gob.NewDecoder(f)
	if err := dec.Decode(&cacheMap); err != nil {
		if errors.Is(err, io.EOF) {
			// Expected!
			return nil
		}
		return err
	}
	return nil
}

// SaveCache writes the cache map to file.
func SaveCache() error {
	if !cacheDirty {
		return nil
	}
	f, err := os.Create(cacheFile)
	if err != nil {
		return err
	}
	defer f.Close()
	cod := gob.NewEncoder(f)
	return cod.Encode(cacheMap)
}

// OptionContracts returns a slice of the option contracts for the ticker.
func (p *PolygonAPI) OptionContracts(ticker string) ([]*tools.Option, error) {
	url := fmt.Sprintf("%s?underlying_ticker=%s", contractURL, ticker)
	req, err := makeRequest(url)
	if err != nil {
		return nil, fmt.Errorf("Couldn't create contracts request %s: %v", url, err)
	}
	res, err := http.DefaultClient.Do(req)
	if res.StatusCode == 429 {
		// Rate limit.
		fmt.Printf("Received %q, pausing before retry\n", res.Status)
		time.Sleep(rateLimitPause)
		res, err = http.DefaultClient.Do(req)
	}
	if err != nil {
		return nil, fmt.Errorf("Bad response: %v", err)
	}
	defer res.Body.Close()
	b, err := io.ReadAll(res.Body)
	if err != nil {
		return nil, fmt.Errorf("Couldn't read response body: %v", err)
	}
	fmt.Printf("Json %s: %s\n", ticker, string(b))
	contracts := &optionContracts{}
	if err := json.Unmarshal(b, contracts); err != nil {
		return nil, fmt.Errorf("Couldn't unmarshal response: %v", err)
	}
	fmt.Printf("Found %d results for %s\n", len(contracts.results), url)
	for _, opt := range contracts.results {
		fmt.Printf("Ticker: %s", opt.underlying)
	}
	return nil, nil
}

// Lookup prints aggregate information about the provided option.
func Lookup(opt *tools.Option, priceDate string) error {
	agg, err := getAgg(opt.String(), priceDate)
	if err != nil {
		return err
	}
	fmt.Printf("Result: %v\n", agg)
	return nil
}
