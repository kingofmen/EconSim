// Package main provides an app to look up options data.
package main

import (
	"encoding/json"
	"flag"
	"fmt"
	"io"
	"log"
	"net/http"
	"time"
)

const (
	apiToken = "lEKU4KMzBOJWda2UaVN7TeOsskQSMBs2"
	apiURL = "https://api.polygon.io/v2/aggs/ticker/O:%s%sC00700000/range/1/day/%s/%s"
)

var (
	tickerF = flag.String("ticker", "TSLA", "Ticker symbol.")
	endDateF = flag.String("enddate", "221021", "Ending date of options.")
	priceDateF = flag.String("pricedate", "", "Date to look up prices.")
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
}

func todayString() string {
	year, month, day := time.Now().Date()
	return fmt.Sprintf("%d-%02d-%d", year, int(month), day)
}

func newURLBuilder() *urlBuilder {
	ub := &urlBuilder{
		ticker: *tickerF,
		priceDate: todayString(),
		endDate: *endDateF,
	}
	if len(*priceDateF) > 0 {
		ub.priceDate = *priceDateF
	}
	return ub
}

func (ub *urlBuilder) makeURL() string {
	return fmt.Sprintf(apiURL, ub.ticker, ub.endDate, ub.priceDate, ub.priceDate)
}

func handlePolyResponse(res *http.Response) error {
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

func makeRequest(url string) (*http.Request, error) {
	req, err := http.NewRequest(http.MethodGet, url, nil)
	if err != nil {
		return nil, err
	}
	req.Header.Set("Authorization", "Bearer " + apiToken)
	return req, nil
}

func main() {
	flag.Parse()
	urlBuilder := newURLBuilder()
	req, err := makeRequest(urlBuilder.makeURL())
	if err != nil {
		log.Fatalf("Couldn't create request: %v", err)
	}
	fmt.Printf("Request: %v\n", req)

	res, err := http.DefaultClient.Do(req)
	if err != nil {
		log.Fatalf("Bad response: %v", err)
	}

	if err := handlePolyResponse(res); err != nil {
		log.Fatalf("Error handling response: %v", err)
	}
}
