package loader

import (
	"os"
	"path/filepath"
	"testing"

	"github.com/google/go-cmp/cmp"
	"google.golang.org/protobuf/encoding/prototext"
	"google.golang.org/protobuf/testing/protocmp"

	cpb "gogames/settlers/economy/chain_proto"
)

func testDataDir(sub string, t *testing.T) string {
	t.Helper()
	src, ok := os.LookupEnv("TEST_SRCDIR")
	if !ok {
		t.Fatal("Could not find TEST_SRCDIR")
	}
	base, ok := os.LookupEnv("TEST_WORKSPACE")
	if !ok {
		t.Fatal("Could not find TEST_WORKSPACE")
	}
	return filepath.Join(src, base, "gogames/settlers/content/testdata", sub)
}

func TestWeb(t *testing.T) {
	path := testDataDir("webs/test*pb.txt", t)
	webs, err := Webs(path)
	if err != nil {
		t.Errorf("Error loading webs: %v", err)
	}
	webMap := make(map[string]*cpb.Web)
	for _, web := range webs {
		webMap[web.GetKey()] = web
	}

	want := []*cpb.Web{
		&cpb.Web{
			Key: "testweb",
			Nodes: []*cpb.Process{
				&cpb.Process{
					Key: "testweb_1",
					Levels: []*cpb.Level{
						&cpb.Level{
							Workers: map[string]int32{"labor": 100, "skilled": 10},
							Outputs: map[string]int32{"grain": 100},
						},
						&cpb.Level{
							Workers: map[string]int32{"labor": 50, "skilled": 50},
							Outputs: map[string]int32{"grain": 250},
						},
					},
				},
				&cpb.Process{
					Key: "testweb_2",
					Levels: []*cpb.Level{
						&cpb.Level{
							Workers: map[string]int32{"admin": 10},
							Outputs: map[string]int32{"grain": 100},
						},
						&cpb.Level{
							Workers: map[string]int32{"clerks": 50},
							Outputs: map[string]int32{"grain": 250},
						},
					},
				},
			},
			Transport: []*cpb.Level{
				&cpb.Level{
					Workers: map[string]int32{"trucks": 10},
				},
				&cpb.Level{
					Workers: map[string]int32{"rail": 10},
				},
			},
		},
	}

	for _, exp := range want {
		got := webMap[exp.GetKey()]
		if got == nil {
			t.Errorf("Did not find web %q", exp.GetKey())
			continue
		}
		if diff := cmp.Diff(got, exp, protocmp.Transform()); len(diff) > 0 {
			t.Errorf("%s: Got %s, want %s, diff %s", exp.GetKey(), prototext.Format(got), prototext.Format(exp), diff)
		}
	}
}
