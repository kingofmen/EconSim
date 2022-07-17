// Package polygon contains methods for interacting with the Polygon.io API.
package polygon

import (
	"encoding/json"
	"fmt"
	"io"
	"net/http"
	"time"

	"trading/tools"
)

const (
	apiToken = "lEKU4KMzBOJWda2UaVN7TeOsskQSMBs2"
	apiURL = "https://api.polygon.io/v2/aggs/ticker/O:%s/range/1/day/%s/%s"
)

// PolygonAPI is a struct for talking to the Polygon finance API. It satisfies FinanceAPI.
type PolygonAPI struct {

}

func (p *PolygonAPI) LookupStock(ticker, date string) (*tools.TickerPrices, error) {
	return nil, nil
}

func (p *PolygonAPI) LookupOption(ticker, priceDate, endDate string, strike int) (*tools.TickerPrices, error) {
	return nil, nil
}


// polyAggregate is a model of the JSON returned by Polygon's 'aggs' endpoint.
type polyAggregate struct {
	Ticker string `json:"ticker"`
	Results []map[string]float64 `json:"results"`
}

func optionsURL(option, date string) string {
	if len(date) == 0 {
		year, month, day := time.Now().Date()
		date = fmt.Sprintf("%d-%02d-%d", year, int(month), day)
	}
	return fmt.Sprintf(apiURL, option, date, date)
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
	agg := &polyAggregate{}
	if err := json.Unmarshal(b, agg); err != nil {
		return err
	}
	fmt.Printf("Result: %v\n", agg)
	return nil
}


// Lookup prints information about the provided option.
func Lookup(opt *tools.Option, priceDate string) error {
	url := optionsURL(opt.String(), priceDate)
	req, err := makeRequest(url)
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
