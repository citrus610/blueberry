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
- Evaluation
  - Material
  - Piece-square table
  - Mobility
- Time management
  - Stops after an iteration if we exceed a soft limit
  - Aborts after exceeding 50% of remaining time
