# Skunk Bot
### Contains:
- Custom chess GUI.
- Rudimentary javascript minimax bot.
- Rudimentary javascript montey carlo bot.
- C bitboard bot (Skunk bot ~1800 ELO).

### C Bitboard bot has implemented:
- Negamax.
- History moves.
- Killer moves.
- Transposition table (Always replacement strategy).
- Quiescence search.
- Check extensions.
- LVA/MVV.
- PV move ordering.
- Verified Null-Move pruning.
- UCI compatible.

### The easiest way to get started with Skunk bot:
- Open terminal.
- Navigate to Chess/skunk_chess_bot/skunk.
- Run ``cmake .`` which should generate Makefile.
- Run ``make`` to compile the bot to your machine.
- Open a chess GUI such as ChessX, open arena, etc.
- Follow software instructions on how to load UCI engine into your GUI.
