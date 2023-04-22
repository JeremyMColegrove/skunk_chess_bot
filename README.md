Skunk Chess Bot
===============

Skunk is a fast and efficient chess bot written in C++ by Jeremy Colegrove. This project was developed for fun and as a learning experience, with a focus on speed and performance. It uses bitboards as the internal data structure for the chess board representation, which allows for efficient move generation and board manipulation. The development of Skunk took approximately 9 months.

Table of Contents
-----------------

- [Skunk Chess Bot](#skunk-chess-bot)
  - [Table of Contents](#table-of-contents)
  - [Getting Started](#getting-started)
  - [Building Skunk](#building-skunk)
  - [Running Skunk](#running-skunk)
  - [Acknowledgements](#acknowledgements)
  - [Universal Chess Interface (UCI) Commands](#universal-chess-interface-uci-commands)

Getting Started
---------------

To use Skunk Chess Bot, first clone the repository:

```bash
git clone https://github.com/username/Skunk.git
cd Skunk
```

## Building Skunk
To build Skunk, make sure you have a C++ compiler and CMake installed. Follow these steps:

1. Create a build directory and navigate to it:
```
mkdir build
cd build
```
2. Generate the build files with CMake:
```
cmake ..
```
3. Compile the project:
```
make
```

## Running Skunk
To run Skunk, execute the binary from the build directory:

```
./Skunk
```

Skunk uses the Universal Chess Interface (UCI) protocol for communication. Refer to the [UCI commands section](https://github.com/JeremyMColegrove/Skunk#universal-chess-interface-uci-commands) for a list of supported commands.

## Acknowledgements
----------------

I would like to thank the open-source community and various resources that have helped me develop Skunk. This project would not have been possible without their valuable insights and guidance.







## Universal Chess Interface (UCI) Commands
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
   - Parameters: depth, movetime
   - Expected response: bestmove [Move]

6. quit
   - Asks the engine to quit the program.
