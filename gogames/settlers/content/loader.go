// Package loader contains functions for loading content from text protobufs.
package loader

import (
	"fmt"
	"os"
	"path/filepath"

	"gogames/settlers/engine/settlers"
	"google.golang.org/protobuf/encoding/prototext"

	cpb "gogames/settlers/economy/chain_proto"
	conpb "gogames/settlers/economy/consumption_proto"
)

func Webs(path string) ([]*cpb.Web, error) {
	webFiles, err := filepath.Glob(filepath.FromSlash(path))
	if err != nil {
		return nil, fmt.Errorf("error reading webs from %s: %w", path, err)
	}

	if len(webFiles) == 0 {
		return nil, fmt.Errorf("no web files found in %q", path)
	}

	webs := make([]*cpb.Web, 0, len(webFiles))
	for _, wf := range webFiles {
		text, err := os.ReadFile(wf)
		if err != nil {
			return nil, fmt.Errorf("error reading web file %s: %w", wf, err)
		}
		web := &cpb.Web{}
		if err := prototext.Unmarshal(text, web); err != nil {
			return nil, fmt.Errorf("error reading web in %s: %w", wf, err)
		}

		webs = append(webs, web)
	}
	return webs, nil
}

func Buckets(path string) ([]*conpb.Bucket, error) {
	return nil, nil
}

func Templates(path string) ([]*settlers.Template, error) {
	return nil, nil
}
