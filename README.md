Skunk Chess Bot
===============

Skunk is a fast and efficient chess bot written in C++ by Jeremy Colegrove. This project was developed for fun and as a learning experience, with a focus on speed and performance. Skunk uses bitboards as the internal data structure for the chess board representation, which allows for efficient move generation and board manipulation. The engine has an Elo rating of around 2400 and features various search optimizations, including Late Move Reductions (LMR), Principal Variation Search (PVS), transposition tables, verified null move pruning, and killer/history heuristic. The development of Skunk took approximately 9 months.

Table of Contents
-----------------

1. [Getting Started](#getting-started)
2. [Building Skunk](#building-skunk)
3. [Running Skunk](#running-skunk)
4. [Acknowledgements](#acknowledgements)

Getting Started
---------------

To use Skunk Chess Bot, first clone the repository:

```bash
git clone https://github.com/username/Skunk.git
cd Skunk
```

Building Skunk
--------------

To build Skunk, make sure you have a C++ compiler and CMake installed. Follow these steps:

1. Create a build directory and navigate to it:

   ```bash
   mkdir build
   cd build
   ```

2. Generate the build files with CMake:

   ```bash
   cmake ..
   ```

3. Compile the project:

   ```bash
   make
   ```

Running Skunk
-------------

To run Skunk, execute the binary from the build directory:

```bash
./Skunk
```

Skunk uses the Universal Chess Interface (UCI) protocol for communication. Refer to the [UCI commands section](https://github.com/username/Skunk#universal-chess-interface-uci-commands) for a list of supported commands.

Acknowledgements
----------------

I would like to thank the open-source community and various resources that have helped me develop Skunk. This project would not have been possible without their valuable insights and guidance.








Universal Chess Interface (UCI) Commands
========================================

<ol>
  <li><strong>uci</strong>
    <ul>
      <li>Tells the engine to use the UCI protocol.</li>
      <li>Expected response:
        <pre>
id name [EngineName]
id author [AuthorName]
(optional: engine options)
uciok
        </pre>
      </li>
    </ul>
  </li>
  <li><strong>isready</strong>
    <ul>
      <li>Asks the engine if it's ready to receive commands.</li>
      <li>Expected response: <code>readyok</code></li>
    </ul>
  </li>
  <li><strong>ucinewgame</strong>
    <ul>
      <li>Informs the engine to start a new game.</li>
      <li>No response expected.</li>
    </ul>
  </li>
  <li><strong>position [fen/startpos] [moves]</strong>
    <ul>
      <li>Sets the current position on the board.</li>
      <li>Use "startpos" for the standard initial position or provide a FEN string for a specific position.</li>
      <li>Optionally, add a sequence of moves (e.g., "e2e4 e7e5") to reach the desired position.</li>
    </ul>
  </li>
  <li><strong>go [parameters]</strong>
    <ul>
      <li>Tells the engine to start calculating the best move.</li>
      <li>Parameters: depth, movetime</li>
      <li>Expected response: <code>bestmove [Move]</code></li>
    </ul>
  </li>
  <li><strong>quit</strong>
    <ul>
      <li>Asks the engine to quit the program.</li>
    </ul>
  </li>
</ol>

