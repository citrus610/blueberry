<div align="center">

  <img src="https://github.com/citrus610/blueberry/blob/main/logo.png" width=12.5% height=12.5%>
  <h1>Blueberry</h1>
  WIP UCI chess engine

</div>

## Features
- Board representation
  - Bitboard
  - Fancy magic bitboard
- Search
  - Iterative deepening
  - Aspiration windows
  - Principle variation search
  - Quiescence search
    - Check evasion
    - Futility pruning
    - SEE pruning
  - Transposition table
    - Depth-preferred with aging replacement scheme
  - Move ordering
    - Hash moves
    - Capture history heuristic
    - SEE
    - Killer moves
    - History heuristic
  - Selectivity
    - Check extensions
    - Reverse futility pruning
    - Null move pruning
    - Late move pruning
    - Futility pruning
    - History pruning
    - SEE pruning
    - Late move reduction
    - Internal iterative reduction
- Evaluation
  - Tapered evaluation
  - Endgame scaling
  - Material
    - Pieces values
    - Bishop pair
  - Piece-square table
  - Mobility
    - Pieces mobility
    - Rooks on open and semi-open files
  - Pawn structure
    - Passed pawns
  - Kings safety
    - King defenders
    - Kings on open and semi-open files
- Time management
  - Soft limit 2% remaining + 50% increment
  - Hard limit 50% remaining

## Thanks
- People in the MinusKelvin & Engine Programming discord server for helping me, they are very cool ❤️
- Disservin's [chess-library](https://github.com/Disservin/chess-library) for helping me understand move generation (I copied most of the movegen code there, bad)
- [Chess programming wiki](https://www.chessprogramming.org/Main_Page) for its resources
