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
  - Transposition table
    - Depth-preferred with aging replacement scheme
  - Move ordering
    - Hash moves
    - MVV/LVA
    - Killer moves
    - History heuristic
  - Extensions
    - Check extensions
  - Pruning
    - Reverse futility pruning
    - Null move pruning
    - Late move pruning
    - Futility pruning
  - Reduction
    - Late move reduction
    - Internal iterative reduction
- Evaluation
  - Material
  - Piece-square table
  - Mobility
- Time management
  - Soft limit 2% remaining + 50% increment
  - Hard limit 50% remaining

## Thanks
- People in the MinusKelvin & Engine Programming discord server for helping me, they are very cool ❤️
- Disservin's [chess-library](https://github.com/Disservin/chess-library) for helping understand move generation
- Chess programming wiki for its resources
