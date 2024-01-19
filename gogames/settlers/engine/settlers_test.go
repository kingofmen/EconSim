package settlers

import (
	"fmt"
	"testing"

	"gogames/tiles/triangles"
)

type testRule struct {
	allow func(bd *Board, fac *Faction, pos ...triangles.TriPoint) []error
}

func (tr *testRule) Allow(bd *Board, fac *Faction, pos ...triangles.TriPoint) []error {
	if tr == nil || tr.allow == nil {
		return nil
	}
	return tr.allow(bd, fac, pos...)
}

func neverRule(s string) *testRule {
	return &testRule{
		allow: func(bd *Board, fac *Faction, pos ...triangles.TriPoint) []error {
			return []error{fmt.Errorf("Never %s!", s)}
		},
	}
}

func TestPlace(t *testing.T) {
	alwaysTmpl := &Template{
		key: "always",
		shape: Shape{
			faces: []Face{
				Face{
					pos:      triangles.TriPoint{0, 0, 0},
					rules:    []Rule{&testRule{}},
					occupied: true,
				},
			},
			verts: []Vert{
				Vert{
					pos:      triangles.TriPoint{0, -1, 0},
					rules:    []Rule{&testRule{}},
					occupied: true,
				},
			},
			faceRules: []Rule{&testRule{}},
			vertRules: []Rule{&testRule{}},
		},
	}

	cases := []struct {
		desc    string
		pos     triangles.TriPoint
		prep    []*Template
		prepPos []triangles.TriPoint
		tmpl    *Template
		errors  []error
		want    map[triangles.TriPoint]Rule
	}{
		{
			desc: "No rules",
			pos:  triangles.TriPoint{0, 0, 1},
			tmpl: &Template{
				key: "norule",
				shape: Shape{
					faces: []Face{
						Face{
							pos:      triangles.TriPoint{0, 0, 0},
							occupied: true,
						},
					},
					verts: []Vert{
						Vert{
							pos:      triangles.TriPoint{0, -1, 0},
							occupied: true,
						},
					},
				},
			},
			want: map[triangles.TriPoint]Rule{
				{0, 0, 1}:  HasKeys("norule"),
				{0, -1, 1}: HasKeys("norule"),
			},
		},
		{
			desc: "Always rule",
			pos:  triangles.TriPoint{0, 0, 1},
			tmpl: alwaysTmpl,
			want: map[triangles.TriPoint]Rule{
				{0, 0, 1}:  HasKeys("always"),
				{0, -1, 1}: HasKeys("always"),
				{1, 0, 1}:  &EmptyRule{},
			},
		},
		{
			desc: "Always rule flip",
			pos:  triangles.TriPoint{1, 0, 1},
			tmpl: alwaysTmpl,
			want: map[triangles.TriPoint]Rule{
				{1, 0, 1}: HasKeys("always"),
				{0, 0, 0}: HasKeys("always"),
				{0, 0, 1}: &EmptyRule{},
			},
		},
		{
			desc: "Nil vertex",
			pos:  triangles.TriPoint{0, 1, 1},
			tmpl: &Template{
				shape: Shape{
					verts: []Vert{
						Vert{
							pos: triangles.TriPoint{0, -1, 100},
						},
					},
				},
			},
			errors: []error{
				fmt.Errorf("vertex position (-1, 1, -100) does not exist"),
			},
		},
		{
			desc: "Nil face",
			pos:  triangles.TriPoint{0, 0, 100},
			tmpl: alwaysTmpl,
			errors: []error{
				fmt.Errorf("tile position (0, 0, 100) does not exist"),
				fmt.Errorf("vertex position (0, -1, 100) does not exist"),
			},
		},
		{
			desc: "Never rule",
			pos:  triangles.TriPoint{0, 0, 1},
			tmpl: &Template{
				key: "never",
				shape: Shape{
					faces: []Face{
						Face{
							pos:      triangles.TriPoint{0, 0, 0},
							rules:    []Rule{neverRule("face")},
							occupied: true,
						},
					},
					verts: []Vert{
						Vert{
							pos:      triangles.TriPoint{0, -1, 0},
							rules:    []Rule{neverRule("vertex")},
							occupied: true,
						},
					},
					faceRules: []Rule{neverRule("triangles")},
					vertRules: []Rule{neverRule("vertices")},
				},
			},
			errors: []error{
				fmt.Errorf("Never face!"),
				fmt.Errorf("Never triangles!"),
				fmt.Errorf("Never vertex!"),
				fmt.Errorf("Never vertices!"),
			},
			want: map[triangles.TriPoint]Rule{
				{0, 0, 1}:  &EmptyRule{},
				{0, -1, 1}: &EmptyRule{},
			},
		},
		{
			desc: "Empty tile rule (face fail)",
			pos:  triangles.TriPoint{0, 0, 1},
			prep: []*Template{alwaysTmpl},
			tmpl: &Template{
				key: "empty",
				shape: Shape{
					faces: []Face{
						Face{
							pos:      triangles.TriPoint{0, 0, 0},
							rules:    []Rule{&EmptyRule{}},
							occupied: true,
						},
					},
				},
			},
			errors: []error{
				fmt.Errorf("position (0, 0, 1) is not empty"),
			},
			want: map[triangles.TriPoint]Rule{
				{0, 0, 1}: HasKeys("always"),
			},
		},
		{
			desc: "Empty tile rule (face succeed)",
			pos:  triangles.TriPoint{0, 0, 1},
			tmpl: &Template{
				key: "empty",
				shape: Shape{
					faces: []Face{
						Face{
							pos:      triangles.TriPoint{0, 0, 0},
							rules:    []Rule{&EmptyRule{}},
							occupied: true,
						},
					},
				},
			},
			want: map[triangles.TriPoint]Rule{
				{0, 0, 1}: HasKeys("empty"),
			},
		},
		{
			desc: "Empty vertex rule (vertex fail)",
			pos:  triangles.TriPoint{0, 0, 1},
			prep: []*Template{alwaysTmpl},
			tmpl: &Template{
				shape: Shape{
					verts: []Vert{
						Vert{
							pos:      triangles.TriPoint{0, -1, 0},
							rules:    []Rule{&EmptyRule{}},
							occupied: true,
						},
					},
				},
			},
			errors: []error{
				fmt.Errorf("position (0, -1, 1) is not empty"),
			},
			want: map[triangles.TriPoint]Rule{
				{0, -1, 1}: HasKeys("always"),
			},
		},
		{
			desc: "Empty vertex rule (vertex succeed)",
			pos:  triangles.TriPoint{0, 0, 1},
			tmpl: &Template{
				key: "empty",
				shape: Shape{
					verts: []Vert{
						Vert{
							pos:      triangles.TriPoint{0, -1, 0},
							rules:    []Rule{&EmptyRule{}},
							occupied: true,
						},
					},
				},
			},
			want: map[triangles.TriPoint]Rule{
				{0, -1, 1}: HasKeys("empty"),
			},
		},
		{
			desc: "Face check does not block face",
			pos:  triangles.TriPoint{0, 0, 1},
			prep: []*Template{
				&Template{
					key: "facecheck",
					shape: Shape{
						faces: []Face{
							Face{
								pos:   triangles.TriPoint{0, 0, 0},
								rules: []Rule{&EmptyRule{}},
							},
						},
						verts: []Vert{
							Vert{
								pos:      triangles.TriPoint{0, -1, 0},
								rules:    []Rule{&EmptyRule{}},
								occupied: true,
							},
						},
					},
				},
			},
			tmpl: &Template{
				key: "emptyface",
				shape: Shape{
					faces: []Face{
						Face{
							pos:      triangles.TriPoint{0, 0, 0},
							rules:    []Rule{&EmptyRule{}},
							occupied: true,
						},
					},
				},
			},
			want: map[triangles.TriPoint]Rule{
				{0, 0, 1}:  HasKeys("emptyface"),
				{0, -1, 1}: HasKeys("facecheck"),
			},
		},
		{
			desc: "Vertex check does not block vertex",
			pos:  triangles.TriPoint{0, 0, 1},
			prep: []*Template{
				&Template{
					key: "vtxcheck",
					shape: Shape{
						faces: []Face{
							Face{
								pos:      triangles.TriPoint{0, 0, 0},
								rules:    []Rule{&EmptyRule{}},
								occupied: true,
							},
						},
						verts: []Vert{
							Vert{
								pos:   triangles.TriPoint{0, -1, 0},
								rules: []Rule{&EmptyRule{}},
							},
						},
					},
				},
			},
			tmpl: &Template{
				key: "emptyvtx",
				shape: Shape{
					verts: []Vert{
						Vert{
							pos:      triangles.TriPoint{0, -1, 0},
							rules:    []Rule{&EmptyRule{}},
							occupied: true,
						},
					},
				},
			},
			want: map[triangles.TriPoint]Rule{
				{0, 0, 1}:  HasKeys("vtxcheck"),
				{0, -1, 1}: HasKeys("emptyvtx"),
			},
		},
		{
			desc: "Has rule (fail)",
			pos:  triangles.TriPoint{0, 0, 1},
			prep: []*Template{alwaysTmpl},
			tmpl: &Template{
				shape: Shape{
					faces: []Face{
						Face{
							pos:      triangles.TriPoint{0, 0, 0},
							rules:    []Rule{HasKeys("always", "always")},
							occupied: true,
						},
					},
				},
			},
			errors: []error{fmt.Errorf("require 2 always, found 1")},
			want: map[triangles.TriPoint]Rule{
				{0, 0, 1}: HasKeys("always"),
			},
		},
		{
			desc: "Has rule (success)",
			pos:  triangles.TriPoint{0, 0, 1},
			prep: []*Template{alwaysTmpl, alwaysTmpl},
			tmpl: &Template{
				shape: Shape{
					faces: []Face{
						Face{
							pos:      triangles.TriPoint{0, 0, 0},
							rules:    []Rule{HasKeys("always", "always")},
							occupied: true,
						},
					},
				},
			},
			want: map[triangles.TriPoint]Rule{
				{0, 0, 1}: HasKeys("always", "always"),
			},
		},
		{
			desc: "Global count (fail)",
			pos:  triangles.TriPoint{0, 0, 1},
			prep: []*Template{alwaysTmpl},
			tmpl: &Template{
				shape: Shape{
					faces: []Face{
						Face{
							pos:      triangles.TriPoint{0, 0, 0},
							occupied: true,
						},
						Face{
							pos:      triangles.TriPoint{0, 1, 0},
							occupied: true,
						},
					},
					faceRules: []Rule{
						HasKeys("always", "always"),
					},
				},
			},
			errors: []error{fmt.Errorf("require 2 always, found 1")},
			want: map[triangles.TriPoint]Rule{
				{0, 0, 1}: HasKeys("always"),
				{0, 1, 1}: &EmptyRule{},
			},
		},
		{
			desc:    "Global count (succeed)",
			pos:     triangles.TriPoint{0, 0, 1},
			prep:    []*Template{alwaysTmpl, alwaysTmpl},
			prepPos: []triangles.TriPoint{{1, 0, 0}},
			tmpl: &Template{
				key: "count",
				shape: Shape{
					faces: []Face{
						Face{
							pos:      triangles.TriPoint{0, 0, 0},
							occupied: true,
						},
						Face{
							pos:      triangles.TriPoint{1, 0, -1},
							occupied: true,
						},
					},
					faceRules: []Rule{
						HasKeys("always", "always"),
					},
				},
			},
			want: map[triangles.TriPoint]Rule{
				{0, 0, 1}: HasKeys("always"),
				{0, 0, 1}: HasKeys("count"),
				{1, 0, 0}: HasKeys("always"),
				{1, 0, 0}: HasKeys("count"),
			},
		},
	}

	for _, cc := range cases {
		t.Run(cc.desc, func(t *testing.T) {
			board, err := NewHex(4)
			if err != nil {
				t.Fatalf("%s: Could not create board: %v", cc.desc, err)
			}
			for i, p := range cc.prep {
				pos := cc.pos
				if len(cc.prepPos) > i {
					pos = cc.prepPos[i]
				}
				if errs := board.Place(pos, nil, p); len(errs) > 0 {
					t.Fatalf("%s: Could not place prep template %d: %v", cc.desc, i, errs)
				}
			}

			errs := board.Place(cc.pos, nil, cc.tmpl)
			if len(errs) != len(cc.errors) {
				t.Errorf("%s: Place() => %v, want %v", cc.desc, errs, cc.errors)
				return
			}
			for idx, err := range errs {
				if err.Error() != cc.errors[idx].Error() {
					t.Errorf("%s: Place() => error %d %q, want %q", cc.desc, idx, err, cc.errors[idx])
				}
			}
			for pos, rule := range cc.want {
				if errs := rule.Allow(board, nil, pos); len(errs) > 0 {
					t.Errorf("%s: Rule fail in %s after placement: %v", cc.desc, pos, errs)
				}
			}
		})
	}
}
