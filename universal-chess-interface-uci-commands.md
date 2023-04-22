
Universal Chess Interface (UCI) Commands
========================================

1. uci
   - Tells the engine to use the UCI protocol.
   - Expected response: 
     id name [EngineName]
     id author [AuthorName]
     (optional: engine options)
     uciok

2. isready
   - Asks the engine if it's ready to receive commands.
   - Expected response: readyok

3. ucinewgame
   - Informs the engine to start a new game.
   - No response expected.

4. position [fen/startpos] [moves]
   - Sets the current position on the board.
   - Use "startpos" for the standard initial position or provide a FEN string for a specific position.
   - Optionally, add a sequence of moves (e.g., "e2e4 e7e5") to reach the desired position.

5. go [parameters]
   - Tells the engine to start calculating the best move.
   - Parameters: depth, nodes, movetime, wtime, btime, winc, binc, movestogo
   - Expected response: bestmove [Move]

6. stop
   - Tells the engine to stop calculating immediately.
   - Expected response: bestmove [Move]

7. quit
   - Asks the engine to quit the program.