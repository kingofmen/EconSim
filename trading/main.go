// Package main provides an app to look up options data.
package main

import (
	"encoding/json"
	"fmt"
	"io"
	"log"
	"net/http"
	"os"
	"time"
)

const (
	apiToken = "lEKU4KMzBOJWda2UaVN7TeOsskQSMBs2"
	apiURL = "https://api.polygon.io/v2/aggs/ticker/O:%s210903C00700000/range/1/day/%s/%s"
)

// PolyAggregate is a model of the JSON returned by Polygon's 'aggs' endpoint.
type PolyAggregate struct {
	Ticker string `json:"ticker"`
	Results []map[string]float64 `json:"results"`
}

type urlBuilder struct {
	ticker string
	date string
}

func todayString() string {
	year, month, day := time.Now().Date()
	return fmt.Sprintf("%d-%02d-%d", year, int(month), day)
}

func newURLBuilder(args []string) *urlBuilder {
	ub := &urlBuilder{
		ticker: "TSLA",
		date: todayString(),
	}

	if len(args) > 0 {
		ub.ticker = args[0]
	}
	if len(args) > 1 {
		ub.date = args[1]
	}
	return ub
}

func (ub *urlBuilder) makeURL() string {
	return fmt.Sprintf(apiURL, ub.ticker, ub.date, ub.date)
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
	urlBuilder := newURLBuilder(os.Args[1:])
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
