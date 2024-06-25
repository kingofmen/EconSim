// Package marketdata defines an interface for the MarketData REST API.
package marketdata

import (
	"encoding/json"
	"fmt"
	"io"
	"net/http"
	"os"
	"strings"
)

var (
	apiToken = ""
)

const (
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

func (q *OptionQuote) String() string {
	if q == nil {
		return "<nil>"
	}
	if len(q.AskSize) == 0 {
		q.AskSize = []float64{0}
	}
	if len(q.AskPrice) == 0 {
		q.AskPrice = []float64{0}
	}
	if len(q.BidSize) == 0 {
		q.BidSize = []float64{0}
	}
	if len(q.BidPrice) == 0 {
		q.BidPrice = []float64{0}
	}
	if len(q.Volume) == 0 {
		q.Volume = []float64{0}
	}
	if len(q.UnderPrice) == 0 {
		q.UnderPrice = []float64{0}
	}
	asize, bsize := q.AskSize[0], q.BidSize[0]
	if asize > 999 {
		asize = 999
	}
	if bsize > 999 {
		bsize = 999
	}
	return fmt.Sprintf("Ask %3.2f / %3.0f Bid %3.2f / %3.0f Volume %f Under %f", q.AskPrice[0], q.AskSize[0], q.BidPrice[0], q.BidSize[0], q.Volume[0], q.UnderPrice[0])
}

func GetOptionQuote(ticker string) (*OptionQuote, error) {
	if ticker != "AAPL" && len(apiToken) == 0 {
		tk, err := os.ReadFile("C:\\Users\\rolfa\\base\\trading\\mdtoken.txt")
		if err != nil {
			return nil, fmt.Errorf("Could not read API token: %v", err)
		}
		apiToken = string(tk)
		apiToken = strings.TrimSpace(apiToken)
	}
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
