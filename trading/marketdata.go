// Package marketdata defines an interface for the MarketData REST API.
package marketdata

import (
	"encoding/json"
	"fmt"
	"io"
	"net/http"
	"strings"
)

const (
	apiToken = ""
	quoteURL = "https://api.marketdata.app/v1/options/quotes/"
)

func makeRequest(url string) (*http.Request, error) {
	req, err := http.NewRequest(http.MethodGet, url, nil)
	if err != nil {
		return nil, err
	}
	if len(apiToken) > 0 {
		req.Header.Set("Authorization", "Bearer "+apiToken)
	}
	return req, nil
}

type OptionQuote struct {
	AskPrice   []float64 `json:"ask"`
	AskSize    []float64 `json:"askSize"`
	BidPrice   []float64 `json:"bid"`
	BidSize    []float64 `json:"bidSize"`
	Volume     []float64 `json:volume"`
	UnderPrice []float64 `json:"underlyingPrice"`
}

func GetOptionQuote(ticker string) (*OptionQuote, error) {
	ticker, _ = strings.CutPrefix(ticker, "O:")
	url := fmt.Sprintf("%s%s", quoteURL, ticker)
	req, err := makeRequest(url)
	if err != nil {
		return nil, fmt.Errorf("Couldn't create quotes request %s: %v", url, err)
	}
	res, err := http.DefaultClient.Do(req)
	if err != nil {
		return nil, fmt.Errorf("Bad response: %v", err)
	}
	defer res.Body.Close()
	b, err := io.ReadAll(res.Body)
	if err != nil {
		return nil, fmt.Errorf("Couldn't read response body: %v", err)
	}
	if !json.Valid(b) {
		return nil, fmt.Errorf("Invalid JSON object")
	}

	quotes := &OptionQuote{}
	if err := json.Unmarshal(b, quotes); err != nil {
		return nil, fmt.Errorf("Couldn't unmarshal response: %v", err)
	}
	return quotes, nil
}
