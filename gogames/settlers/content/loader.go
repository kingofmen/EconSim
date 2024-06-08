// Package loader contains functions for loading content from text protobufs.
package loader

import (
	"fmt"
	"os"
	"path/filepath"

	"gogames/settlers/engine/settlers"
	"google.golang.org/protobuf/encoding/prototext"
	"google.golang.org/protobuf/proto"

	cpb "gogames/settlers/economy/chain_proto"
	conpb "gogames/settlers/economy/consumption_proto"
)

func loadProtos(path string, make func() proto.Message) error {
	files, err := filepath.Glob(filepath.FromSlash(path))
	if err != nil {
		return fmt.Errorf("error reading files from %s: %w", path, err)
	}

	if len(files) == 0 {
		return fmt.Errorf("no proto files found in %q", path)
	}

	for _, wf := range files {
		text, err := os.ReadFile(wf)
		if err != nil {
			return fmt.Errorf("error reading file %s: %w", wf, err)
		}
		target := make()
		if err := prototext.Unmarshal(text, target); err != nil {
			return fmt.Errorf("error reading text proto in %s: %w", wf, err)
		}
	}
	return nil
}

func Webs(path string) ([]*cpb.Web, error) {
	webs := make([]*cpb.Web, 0, 16)
	if err := loadProtos(path, func() proto.Message {
		web := &cpb.Web{}
		webs = append(webs, web)
		return web
	}); err != nil {
		return nil, err
	}
	return webs, nil
}

func Buckets(path string) ([]*conpb.Bucket, error) {
	return nil, nil
}

func Templates(path string) ([]*settlers.Template, error) {
	return nil, nil
}
