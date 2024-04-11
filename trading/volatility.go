// Package main simulates volatility trades.
package main

import (
	"flag"
	"fmt"
	"math"
	"math/rand"
)

var (
	trueVol    = flag.Float64("vol", 0.0, "True volatility")
	los        = flag.Float64("lost", 0.0, "Percentage increase of the low strike price.")
	his        = flag.Float64("hist", 0.0, "Percentage increase of the high strike price.")
	startPrice = flag.Float64("start", 100.0, "Starting price.")
	loPremium  = flag.Float64("lopr", 0.0, "Low-strike premium.")
	hiPremium  = flag.Float64("hipr", 0.0, "High-strike premium.")
)

// step returns a random yield with volatility (stddev of log return) equal to vol.
func step(vol float64) float64 {
	logRet := rand.NormFloat64() * vol
	return math.Exp(logRet)
}

// simulate returns the average return (in dollars, ignoring premiums)
// of buying a call at loStrike and selling at hiStrike, given true
// volatility vol.
func simulate(start, vol, loStrike, hiStrike float64) float64 {
	avg := float64(0.0)
	cycles := 100
	for cc := 0; cc < cycles; cc++ {
		// Do intermediate prices matter?
		final := start * step(vol)
		if final > loStrike {
			avg += final - loStrike
		}
		if final > hiStrike {
			avg -= final - hiStrike
		}
	}

	return avg / float64(cycles)
}

func main() {
	flag.Parse()

	// Buy a call at low strike, sell a call at high strike.
	val := simulate(*startPrice, *trueVol, *startPrice*(1+*los), *startPrice*(1+*his))
	val -= *loPremium
	val += *hiPremium
	fmt.Printf("%.2f\n", val)
}
