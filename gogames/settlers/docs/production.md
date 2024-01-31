## Production logic

The economic logic is the backbone of the engine, the thing that the
player attempts to affect through placing pieces; the turn-by-turn
output of the tiles determines player score. Here I give the broad
outline of how it works.

### Things humans want and make

Settlers models all economic activity as being roughly divided into
four parts: Consumption, meaning, investment, and military. (I suppose
one *could* map that onto fire/air/earth/water or
Hufflepuff/Ravenclaw/Slytherin/Gryffindor if one *really* wanted to.)
Briefly:

1. Consumption is food, clothes, shelter, fuel, spices, televisions,
   and smartphones: All the material goods that make life possible,
   comfortable, or pleasant. People with zero consumption are dead;
   with low consumption, starving and unable to think of anything
   beyond their next meal; with high consumption, wealthy.
2. Meaning is art, beauty, worship, narrative, seeing one's children
   grow up, ritual, sport, and identity: The non-material things
   (although sometimes expressed in objects) that give life
   meaning. People with low meaning complain of empty lives, of feeling
   that there "must be something more"; flock to any cause or
   guru who will set up a narrative they can believe in; buy material
   things they don't really want, and fling them in the nearest closet
   because what was the point? People with high meaning feel there is
   a purpose to their lives, however harsh; have many children that
   can carry on the work; look back from their old age and say "that
   was a good life".
3. Investment is tools, fences, machinery, roads and bridges, metis
   and praxis, education and apprenticeship, inventing and building -
   anything that makes it easier to work, that produces more of
   something for the same effort. People with low investment perform
   backbreaking repetitive labour daily, and struggle to live off the
   meager results; people with high investment operate industrial
   civilisations from high-finish control panels, and complain if
   their work shift extends beyond an hour because "what are robots
   for anyway?"
4. Military is swords and guns, armour and horses, militia training
   and press gangs, city walls and border patrols,
   watchtowers and beacons and war-arrows that run from farm to farm;
   it is the rough men who stand guard so that others may sleep warm
   in their beds, and all the potential violence that keeps the other three
   categories safe from predation. People with no military are overrun
   by barbarians internal or external, or have their goods stolen with
   no recourse; people with high military... tend, if we're honest, to
   go looking for someone they can steal from, because why did they
   put in all that effort getting strong, if not to use the strength
   for something? And then they underestimate someone and find that,
   actually, they'd have been better off with some investment in
   productive assets. But there's no prize for the second-best
   military.

### Subdivisions

In addition to the four ~~elements~~ ~~Houses~~ areas, resources are
divided into 'buckets', sometimes called goods. Each good has a
desired amount, to be consumed each turn, and each good has some
effects, which may be triggered by being below a minimum, by reaching
the cap, or linearly in the percentage of the cap reached. Further,
goods can have prerequisite goods; for example, no human is likely to
consume any art or spirituality while eating 500 calories a day - so
the "starvation diet" bucket must be filled up before any other goods
can be produced. One can think of production as a sort of fountain of
buckets, with labor and capital pouring first into the one labeled
"bare survival" and, if there's enough, overflowing to fill the other
buckets in sequence.

Some goods are stockpilable, and do not fully vanish each turn;
instead they have a decay rate, and the steady-state is to produce
enough of them to counter the decay. For example, surplus in the
'food' bucket is "stored as fat", which can be drawn upon to alleviate
a future shortage - until it's gone.

### Production decisions

Each turn, each piece produces some amount of labour, which it then
turns into goods. For each unit of labour, the piece considers what
buckets are available to it, that is, have their requirements filled;
it prioritises those buckets and uses the labour to put something in
the most important one. The amount is given by the production function
for that bucket, which depends on the total labour used, the capital
available for it, and any modifiers the piece itself may have. 
The first unit of labour is highly likely to go to the
"starvation diet" bucket, in other words, basic survival needs; once
that's full, other buckets such as spirituality, comfort, and
safety become available and will be filled in order of their priority.

A bucket is prioritised by:

1. Increase in closeness of the four-areas allocation to the target;
   in other words, if all four areas are equally important to this
   piece, and military has a score of zero from the current effects of
   filled buckets, then a military bucket will be prioritised if
   possible.
2. Absolute output.
3. Definition order.

### Trade

Some production functions are actually trade with other pieces; this
is considered in a separate doc.

