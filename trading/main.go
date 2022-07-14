// Package main provides an app to look up options data.
package main

import (
	"encoding/json"
	"fmt"
	"io"
	"log"
	"net/http"
	"os"
)

const (
	apiToken = "lEKU4KMzBOJWda2UaVN7TeOsskQSMBs2"
	apiURL = "https://api.polygon.io/v2/aggs/ticker/O:%s210903C00700000/range/1/day/2021-07-22/2021-07-22"
)

// PolyAggregate is a model of the JSON returned by Polygon's 'aggs' endpoint.
type PolyAggregate struct {
	Ticker string `json:"ticker"`
	Results []map[string]float64 `json:"results"`
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

func makeURL(ticker string) string {
	return fmt.Sprintf(apiURL, ticker)
}

func makeRequest(ticker string) (*http.Request, error) {
	req, err := http.NewRequest(http.MethodGet, makeURL(ticker), nil)
	if err != nil {
		return nil, err
	}
	req.Header.Set("Authorization", "Bearer " + apiToken)
	return req, nil
}

func main() {
	ticker := "TSLA"
	if len(os.Args) > 1 {
		ticker = os.Args[1]
	}

	req, err := makeRequest(ticker)
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
