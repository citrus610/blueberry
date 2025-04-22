# blueberry
UCI chess engine WIP

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
- Evaluation
  - Material
  - Piece-square table
  - Mobility
- Time management
  - Stops after an iteration if we exceed a soft limit
  - Aborts after exceeding 50% of remaining time
