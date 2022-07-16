// Package polygon contains methods for interacting with the Polygon.io API.
package polygon

import (
	"encoding/json"
	"fmt"
	"io"
	"net/http"
	"time"
)

const (
	apiToken = "lEKU4KMzBOJWda2UaVN7TeOsskQSMBs2"
	apiURL = "https://api.polygon.io/v2/aggs/ticker/O:%s%s%s%08d/range/1/day/%s/%s"
)

// PolyAggregate is a model of the JSON returned by Polygon's 'aggs' endpoint.
type PolyAggregate struct {
	Ticker string `json:"ticker"`
	Results []map[string]float64 `json:"results"`
}

type urlBuilder struct {
	ticker string
	priceDate string
	endDate string
	call bool
	priceDollars int
}

func todayString() string {
	year, month, day := time.Now().Date()
	return fmt.Sprintf("%d-%02d-%d", year, int(month), day)
}

func newURLBuilder(ticker, priceDate, endDate string, strike int) *urlBuilder {
	ub := &urlBuilder{
		ticker: ticker,
		priceDate: todayString(),
		endDate: endDate,
		call: true,
		priceDollars: strike,
	}
	if len(priceDate) > 0 {
		ub.priceDate = priceDate
	}
	return ub
}

func (ub *urlBuilder) optionType() string {
	if ub.call {
		return "C"
	}
	return "P"
}

func (ub *urlBuilder) makeURL() string {
	return fmt.Sprintf(apiURL, ub.ticker, ub.endDate, ub.optionType(), ub.priceDollars*1000, ub.priceDate, ub.priceDate)
}

func makeRequest(url string) (*http.Request, error) {
	req, err := http.NewRequest(http.MethodGet, url, nil)
	if err != nil {
		return nil, err
	}
	req.Header.Set("Authorization", "Bearer " + apiToken)
	return req, nil
}

func printResponse(res *http.Response) error {
	defer res.Body.Close()
	fmt.Printf("Response: %v\n", res)
	b, err := io.ReadAll(res.Body)
	if err != nil {
		return err
	}
	agg := &PolyAggregate{}
	if err := json.Unmarshal(b, agg); err != nil {
		return err
	}
	fmt.Printf("Result: %v\n", agg)
	return nil
}


// Lookup prints information about the provided option.
func Lookup(ticker, endDate, priceDate string, strike int) error {
	urlBuilder := newURLBuilder(ticker, endDate, priceDate, strike)
	req, err := makeRequest(urlBuilder.makeURL())
	if err != nil {
		return fmt.Errorf("Couldn't create request: %v", err)
	}
	fmt.Printf("Request: %v\n", req)

	res, err := http.DefaultClient.Do(req)
	if err != nil {
		return fmt.Errorf("Bad response: %v", err)
	}

	if err := printResponse(res); err != nil {
		return fmt.Errorf("Error handling response: %v", err)
	}

	return nil
}
