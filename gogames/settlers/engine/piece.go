package settlers

import (
	"gogames/tiles/triangles"
)

type Activity int

const (
	CONSUME = iota // Eat, drink, and be merry; all that feeds the body.
	MEANING        // Ritual, sport, family, worship; all that makes life worth living.
	PRODUCE        // Investment, building, the creation of tools.
	MILITIA        // Drill, weapons, training, patrol; the defense of the other three.
)

// Edge contains occupation information for a triangle edge.
type Edge struct {
	dir   triangles.Direction
	rules []Rule
}

// Vert contains occupation information for a triangle vertex.
type Vert struct {
	// Tri-coordinates relative to base face.
	pos triangles.TriPoint
	// Vertex-specific rules.
	rules []Rule
}

// Face contains occupation information for one triangle tile.
type Face struct {
	// Tri-coordinates relative to base face.
	pos triangles.TriPoint
	// Rules for the face.
	rules []Rule
	// Occupied edges.
	edges []Edge
}

// Shape is a collection of Faces.
type Shape struct {
	faces []Face
	verts []Vert
	// Rules for all occupied faces.
	faceRules []Rule
	// Rules for all edges.
	edgeRules []Rule
	// Rules for all vertices.
	vertRules []Rule
}

// Template contains information shared across a class of game pieces,
// such as shape and production capability.
type Template struct {
	people int64
	shape  Shape
}

// Piece is an instantiation of a Template, placed on the board.
type Piece struct {
	kind       *Template
	faction    *Faction
	priorities [4]int16
	people     int64
}

func NewPiece(tmp *Template, fac *Faction) *Piece {
	return &Piece{
		kind:       tmp,
		faction:    fac,
		priorities: [4]int16{250, 250, 250, 250},
		people:     0,
	}
}

func (s *Piece) Populate() {
	if s == nil {
		return
	}
	if s.faction == nil {
		return
	}

	want := s.kind.people - s.people
	if s.faction.Freemen < want {
		want = s.faction.Freemen
	}
	s.people += want
	s.faction.Freemen -= want
}

// Prioritize calculates the production fractions.
func (s *Piece) Prioritize() {
	if s == nil {
		return
	}
	// TODO: Replace this placeholder stub with an actual calculation!
	s.priorities[CONSUME] = 250
	s.priorities[MEANING] = 250
	s.priorities[PRODUCE] = 250
	s.priorities[MILITIA] = 250
}

func (s *Piece) Produce() {
	if s == nil {
		return
	}
}
