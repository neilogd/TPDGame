-- Run psybrus scripts.
dofile( "Psybrus/Scripts/Psybrus.lua" )

-- Solution.
PsySolutionGame( GAME.name )

-- Build externals.
dofile( "External/genie.lua" )
dofile( "Psybrus/External/genie.lua" )

-- Build engine.
dofile( "Psybrus/Engine/genie.lua" )

-- Build game source.
dofile( "Source/genie.lua" )
