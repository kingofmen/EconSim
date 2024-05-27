package settlers

import (
	"fmt"

	"gogames/settlers/economy/chain"
	"gogames/tiles/triangles"
	"gogames/util/counts"

	cpb "gogames/settlers/economy/chain_proto"
	conpb "gogames/settlers/economy/consumption_proto"
)

type Tile struct {
	*triangles.Surface
	*chain.Location
	pieces []*Piece
}

type Point struct {
	*triangles.Vertex
	pieces []*Piece
}

// Board contains the game board.
type Board struct {
	Tiles   []*Tile
	Points  []*Point
	tileMap map[triangles.TriPoint]*Tile
	vtxMap  map[triangles.TriPoint]*Point
	pieces  []*Piece
}

// NewHex returns a six-sided board of radius r.
func NewHex(r int) (*Board, error) {
	if r < 1 {
		return nil, fmt.Errorf("hex board of radius %d would be empty", r)
	}
	b := &Board{
		// Hex consists of 6 large triangles; hence the number of tiles is 6
		// times the nth triangular number.
		Tiles:   make([]*Tile, 0, 3*r*(r+1)),
		tileMap: make(map[triangles.TriPoint]*Tile),
		vtxMap:  make(map[triangles.TriPoint]*Point),
		pieces:  make([]*Piece, 0, 10),
	}

	surfaces := make([]*triangles.Surface, 0, len(b.Tiles))
	for warp := r; warp > -r; warp-- {
		for weft := r; weft > -r; weft-- {
			for hex := r; hex > -r; hex-- {
				s, err := triangles.New(warp, weft, hex)
				if err != nil {
					// Invalid tripoint.
					continue
				}
				surfaces = append(surfaces, s)
				tile := &Tile{
					Surface:  s,
					Location: chain.NewLocation(),
					pieces:   make([]*Piece, 0, 3),
				}
				b.Tiles = append(b.Tiles, tile)
				b.tileMap[s.GetTriPoint()] = tile
			}
		}
	}

	if err := triangles.Tile(surfaces...); err != nil {
		return nil, err
	}

	// Vertices are created and given positions by Tile.
	for _, tile := range b.Tiles {
		for _, vtx := range tile.GetVertices() {
			if _, seen := b.vtxMap[vtx.GetTriPoint()]; seen {
				continue
			}
			point := &Point{
				Vertex: vtx,
				pieces: make([]*Piece, 0, 3),
			}
			b.vtxMap[vtx.GetTriPoint()] = point
			b.Points = append(b.Points, point)
		}
	}

	return b, nil
}

// Pieces returns the pieces on the tile.
func (t *Tile) Pieces() []*Piece {
	if t == nil {
		return nil
	}
	return t.pieces
}

// prioritise returns the best candidate to consume.
func (t *Tile) prioritise(buckets []*conpb.Bucket) *conpb.Bucket {
	if len(buckets) == 0 {
		return nil
	}
	// TODO: Implement me.
	return buckets[0]
}

// Consume uses resources and scores points.
func (t *Tile) Consume(buckets []*conpb.Bucket) {
	if t == nil {
		return
	}
	filled := map[string]bool{}
	discard := map[string]bool{}
	for {
		cands := make([]*conpb.Bucket, 0, len(buckets))
		for _, bucket := range buckets {
			if filled[bucket.GetKey()] {
				continue
			}
			if discard[bucket.GetKey()] {
				continue
			}
			for _, req := range bucket.GetPrereqs() {
				if discard[req] {
					discard[bucket.GetKey()] = true
					continue
				}
				if !filled[req] {
					continue
				}
			}
			// TODO: Scale by population!
			if !counts.SuperInt32(t.Location.Goods, bucket.GetStuff()) {
				discard[bucket.GetKey()] = true
				continue
			}
			cands = append(cands, bucket)
		}
		if len(cands) == 0 {
			break
		}
		bucket := t.prioritise(cands)
		counts.SubtractInt32(t.Location.Goods, bucket.GetStuff())
		filled[bucket.GetKey()] = true
	}
}

// Controller returns the controlling faction.
func (t *Tile) Controller() *Faction {
	if t == nil {
		return nil
	}
	if len(t.pieces) < 1 {
		return nil
	}
	count := make(map[*Faction]int)
	best := t.pieces[0].GetFaction()
	count[best] = t.pieces[0].getWeight()
	for _, piece := range t.pieces[1:] {
		fac := piece.GetFaction()
		count[fac] += piece.getWeight()
		if count[fac] > count[best] {
			best = fac
		}
	}
	return best
}

// Pieces returns the pieces on the vertex.
func (v *Point) Pieces() []*Piece {
	if v == nil {
		return nil
	}
	return v.pieces
}

// CheckShape returns any rules conflicts of placing the piece.
func (bd *Board) CheckShape(pos triangles.TriPoint, fac *Faction, tmp *Template, turns Rotate) []error {
	bad := make([]error, 0, len(tmp.shape.faces))
	facePos := make([]triangles.TriPoint, 0, len(tmp.shape.faces))
	for _, face := range tmp.shape.faces {
		coord := face.From(pos, turns)
		if _, ok := bd.tileMap[coord]; !ok {
			bad = append(bad, fmt.Errorf("tile position %s does not exist", coord))
			continue
		}
		facePos = append(facePos, coord)
		for _, rule := range face.rules {
			if errs := rule.Allow(bd, fac, coord); len(errs) > 0 {
				bad = append(bad, errs...)
			}
		}
	}
	for _, rule := range tmp.shape.faceRules {
		if errs := rule.Allow(bd, fac, facePos...); len(errs) > 0 {
			bad = append(bad, errs...)
		}
	}

	vertPos := make([]triangles.TriPoint, 0, len(tmp.shape.faces))
	for _, vert := range tmp.shape.verts {
		coord := vert.From(pos, turns)
		if _, ok := bd.vtxMap[coord]; !ok {
			bad = append(bad, fmt.Errorf("vertex position %s does not exist", coord))
			continue
		}
		vertPos = append(vertPos, coord)
		for _, rule := range vert.rules {
			if errs := rule.Allow(bd, fac, coord); len(errs) > 0 {
				bad = append(bad, errs...)
			}
		}
	}
	for _, rule := range tmp.shape.vertRules {
		if errs := rule.Allow(bd, fac, vertPos...); len(errs) > 0 {
			bad = append(bad, errs...)
		}
	}

	// TODO: Edges.

	return bad
}

// PlaceParams contains parameters for the Place function.
type PlaceParams struct {
	pos   triangles.TriPoint
	fac   *Faction
	tmp   *Template
	turns Rotate
}

// Valid returns an error if required parameters are not present.
func (pp *PlaceParams) Valid() error {
	if pp == nil {
		return fmt.Errorf("invalid place parameters: nil")
	}
	if err := pp.pos.ValidSurface(); err != nil {
		return fmt.Errorf("invalid place parameters: %w", err)
	}
	if pp.tmp == nil {
		return fmt.Errorf("invalid place parameters: template is nil")
	}
	return nil
}

func NewPlacement(pos triangles.TriPoint, tmp *Template) *PlaceParams {
	return &PlaceParams{
		pos: pos,
		tmp: tmp,
	}
}

func (pp *PlaceParams) WithFaction(fac *Faction) *PlaceParams {
	if pp == nil {
		return nil
	}
	pp.fac = fac
	return pp
}

func (pp *PlaceParams) WithRotation(turns Rotate) *PlaceParams {
	if pp == nil {
		return nil
	}
	pp.turns = turns
	return pp
}

// Place puts a Piece onto the board.
func (bd *Board) Place(pp *PlaceParams) []error {
	if bd == nil {
		return []error{fmt.Errorf("piece placed on nil Board")}
	}
	if err := pp.Valid(); err != nil {
		return []error{err}
	}

	if errors := bd.CheckShape(pp.pos, pp.fac, pp.tmp, pp.turns); len(errors) > 0 {
		return errors
	}

	p := NewPiece(pp.tmp, pp.fac)
	bd.pieces = append(bd.pieces, p)
	for _, face := range pp.tmp.shape.faces {
		if !face.occupied {
			continue
		}
		coord := face.From(pp.pos, pp.turns)
		tile := bd.tileMap[coord]
		tile.pieces = append(tile.pieces, p)
	}
	for _, vert := range pp.tmp.shape.verts {
		if !vert.occupied {
			continue
		}
		coord := vert.From(pp.pos, pp.turns)
		point := bd.vtxMap[coord]
		point.pieces = append(point.pieces, p)
	}

	return nil
}

// TickParams contains non-Board information needed to run a turn.
type TickParams struct {
	Webs    []*cpb.Web
	Buckets []*conpb.Bucket
}

func NewTick() *TickParams {
	return &TickParams{}
}

func (tp *TickParams) WithWebs(w []*cpb.Web) *TickParams {
	if tp == nil {
		tp = NewTick()
	}
	tp.Webs = w
	return tp
}

func (tp *TickParams) WithBuckets(b []*conpb.Bucket) *TickParams {
	if tp == nil {
		tp = NewTick()
	}
	tp.Buckets = b
	return tp
}

// Tick runs all economic and military logic for one timestep.
func (m *Board) Tick(params *TickParams) error {
	if m == nil {
		return fmt.Errorf("Tick called on nil Board")
	}

	asLocs := make([]*chain.Location, 0, len(m.Tiles))
	for _, tile := range m.Tiles {
		if tile == nil {
			continue
		}
		asLocs = append(asLocs, tile.Location)
		for _, piece := range tile.pieces {
			tile.Location.MakeAvailable(piece.GetWorkers())
		}
	}

	for {
		if cmb := chain.Place(params.Webs, asLocs); cmb != nil {
			cmb.Activate()
			continue
		}
		break
	}
	for _, tile := range m.Tiles {
		tile.Location.Work()
	}

	// TODO: Trade goes here.

	for _, tile := range m.Tiles {
		tile.Consume(params.Buckets)
	}

	return nil
}

func (bd *Board) GetPieces(loc triangles.TriPoint) ([]*Piece, error) {
	if bd == nil {
		return nil, fmt.Errorf("cannot get pieces from nil Board")
	}
	if tile := bd.tileMap[loc]; tile != nil {
		return tile.pieces, nil
	}
	if point := bd.vtxMap[loc]; point != nil {
		return point.pieces, nil
	}
	return nil, fmt.Errorf("position %s is not on the board", loc)
}

// GetTile returns the tile at the given location, if there is one.
func (bd *Board) GetTile(loc triangles.TriPoint) *Tile {
	if bd == nil {
		return nil
	}
	return bd.tileMap[loc]
}
