class Node {
    constructor(move, maximize) {
        this.UCB1 = -Infinity
        this.move = move    //The particular action at this node
        this.t = 0          //The total score summed from previously visited nodes
        this.n = 0          //The number of times this node has been visited
        this.root = null    //This nodes root node (useful for backpropogation)
        this.children = []  //This nodes children (used for expanding)
        this.c = 0.5

        this.maximize = maximize
        if (!maximize) this.UCB1 = Infinity
    }

    refreshUCB1() {
        if (this.n <= 0) return

        if (this.maximize)
        {
            this.UCB1 = (this.t/this.n) - this.c * Math.sqrt(Math.log(this.root.n) / this.n)
        }
        else
        {
            this.UCB1 = (this.t/this.n) + this.c * Math.sqrt(Math.log(this.root.n) / this.n)
        }
    }
}



class MonteCarlo {
    constructor (iterations) {
        this.iterations = iterations    //How long will we let the simulation run for?
        this.root = null                //The root state to try and figure out
        this.current = null             //The current node our tree is on
        this.endgame = false
    }

    playMove(pause=null) {
        if (pause) 
        {
            setTimeout(()=>this.playMove(), pause)
            return
        }

        let pieces = board.getPieceCount()
        if (!this.endgame && pieces[0] + pieces[1] < 20) 
        {
            console.warn("Engine is switching to endgame strategy")
            this.endgame = true
        }


        //This represents the move white just made, not blacks
        this.root = new Node(null, board.turn)
        this.current = this.root

        let move = this.run()

        if (!move) {
            console.log("Found no move. This probably means you have checkmate! Goodgame!")
            return
        }

        board.move(move)
        board.turn = !board.turn
        board.step()

        //Don't get rid of our previous results
        this.root = null
        this.current = null
    }

    run() {
        let iterations = 0
        while (iterations++<this.iterations)
        {
            this.selection()
            this.expansion()
            let v = this.rollout()
            this.backpropagate(v)
        }
        // console.log(this.current.children)
        let best = null
        for (var node of this.root.children) {
            if (!best || node.n > best.n) best = node
        }
        console.log(this.root)
        return best.move
    }

    //Navigate as far into the tree as we can, until we reach a leaf node that has not been visited yet
    selection(depth = 0) {
        if (depth > 40) return 
        let next = this.getLargestChild()
        //This means the root node is a leaf node, so we just need to expand it
        if (next == null) return

        this.navigateInto(next)
        if (this.current.n > 0)
        {
            this.selection(depth + 1)
        }
    }

    //Expand the current moves
    expansion() 
    {
        //Add in available actions for the current node
        if (!this.current.children.length == 0) return
        for (var entry of board.valid_moves) 
        {
            for (var move of entry[1]) 
            {
                let new_node = new Node(move, !this.current.maximize)
                new_node.root = this.current
                this.current.children.push(new_node)
            }
        }
    }

    //Use a deterministic function to evaluate board positions instead of random walk
    rollout() {
        return this.staticEvaluation()
    }

    backpropagate(value) 
    {
        let depth = 0
        while (this.current.root != null) 
        {
            depth++
            
            //Only add the value if it is a win for black
            // if (value < 0 && !this.maximize) this.current.t += -value
            // else if (value>0 && this.maximize) 
            this.current.t += value
            this.current.n += 1

            board.undo()
            board.turn = !board.turn
            this.current = this.current.root
        }
        // console.log(depth)
        board.step(true)
        this.current.n += 1
    }
    
    // staticEvaluation() 
    // {
    //     //How many pieces does each side have
    //     let w_piece_count = new Map()
    //     let b_piece_count = new Map()

    //     //How many available moves does each player have in this position
    //     let w_moves = 0
    //     let b_moves = 0

    //     //Save the current turn
    //     let old_turn = board.turn

    //     //Check whites available moves
    //     board.turn = 1
    //     board.step(true)
    //     w_moves = Array.from(board.valid_moves, ([name, value]) => value).flat().length;

    //     //Check blacks available moves
    //     board.turn = 0
    //     board.step(true)
    //     b_moves = Array.from(board.valid_moves, ([name, value]) => value).flat().length;
        
    //     //Switch back to the old turn
    //     board.turn = old_turn
    //     board.step(true)

    //     if (w_moves == 0) return -100
    //     if (b_moves == 0) return 100


    //     //Go through the board and count each players available pieces
    //     w_piece_count.set("King", 0)
    //     w_piece_count.set("Queen", 0)
    //     w_piece_count.set("Rook", 0)
    //     w_piece_count.set("Pawn", 0)
    //     w_piece_count.set("Bishop", 0)
    //     w_piece_count.set("Knight", 0)
    //     b_piece_count.set("King", 0)
    //     b_piece_count.set("Queen", 0)
    //     b_piece_count.set("Rook", 0)
    //     b_piece_count.set("Pawn", 0)
    //     b_piece_count.set("Bishop", 0)
    //     b_piece_count.set("Knight", 0)
    //     let center = 0
    //     for (var i=0; i<board.board[0].length;i++)
    //     {
    //         for (var j=0; j<board.board.length; j++)
    //         {
    //             let square = board.getSquare(i, j)
    //             if (!square.piece) continue
    //             let name = square.piece.getName()

    //             if (square.piece.color)
    //             {
    //                 center += 0.1*  Math.sin(square.x*Math.PI/8)*Math.sin(square.y*Math.PI/8)
    //                 w_piece_count.set(name, w_piece_count.get(name) + 1)
    //             } else
    //             {
    //                 center -= 0.1*  Math.sin(square.x*Math.PI/8)*Math.sin(square.y*Math.PI/8)
    //                 b_piece_count.set(name, b_piece_count.get(name) + 1)
    //             }
                
    //         }
    //     }
    //     // console.log(w_piece_count, b_piece_count, w_moves, b_moves)

    //     let assessment =    200*(w_piece_count.get("King") - b_piece_count.get("King"))
    //                     +   9*  (w_piece_count.get("Queen") - b_piece_count.get("Queen"))
    //                     +   5*  (w_piece_count.get("Rook") - b_piece_count.get("Rook"))
    //                     +   3*  (w_piece_count.get("Bishop") - b_piece_count.get("Bishop"))
    //                     +   1*  (w_piece_count.get("Pawn")  -   b_piece_count.get("Pawn"))
    //                     // Mobility
    //                     +   .02*(w_moves - b_moves)
    //                     //Center control
    //                     +   0.2*center
        
    //     // console.log(assessment)
    //     return assessment
        // return 0.5
        /**
         * f(p) = 200(K-K')
       + 9(Q-Q')
       + 5(R-R')
       + 3(B-B' + N-N')
       + 1(P-P')
       - 0.5(D-D' + S-S' + I-I')
       + 0.1(M-M') + ...

        KQRBNP = number of kings, queens, rooks, bishops, knights and pawns
        D,S,I = doubled, blocked and isolated pawns
        M = Mobility (the number of legal moves)


        OR
        materialScore = kingWt  * (wK-bK)
                    + queenWt * (wQ-bQ)
                    + rookWt  * (wR-bR)
                    + knightWt* (wN-bN)
                    + bishopWt* (wB-bB)
                    + pawnWt  * (wP-bP)

        mobilityScore = mobilityWt * (wMobility-bMobility)

        Score = materialscore + mobility score
         */
        
        // //Count the pieces on the board
        // let assessment = 0        
        // //Assess by piece value
        // for (var i=0; i<board.board[0].length; i++) 
        // {
        //     for (var j=0; j<board.board.length; j++) 
        //     {
        //         let square = board.getSquare(i, j)
        //         if (!square.piece) continue
        //         let piece = square.piece

        //         //This just moves one piece to the center very often, instead of that lets go for number of pieces in the center
        //         let center = Math.sin(piece.x*Math.PI/8)*Math.sin(piece.y*Math.PI/8)*10
                
        //         //If in the endgame, try to keep the king in the center of the board
        //         //This also tries to move the opponents king to the edge of the board
        //         if (piece instanceof King)
        //         {
        //             if (this.endgame) 
        //             {
        //                 //Weight it very heavily as being towards the center
        //                 center *= 2
        //             } else center = 0
        //         } else if (this.endgame)
        //         {
        //             center = 0
        //         }
                

        //         //Try to push pawns to the end of the board
        //         if (piece instanceof Pawn) 
        //         {
        //             if (this.endgame)
        //             {
        //                 center = Math.abs(Math.cos(piece.y*Math.PI/7))*20
        //             }
        //         }
            
            

        //         let push_pawns = 0

        //         let points = piece.points*100
        //         //Weight the queen as an even higher piece (to avoid sacrafices)
        //         if (piece instanceof Queen) points *= 2

        //         let developed = (!(piece instanceof Pawn) && !(piece instanceof Rook) && piece.moves < 1) ? -5 : 0
        //         let castled = (!this.endgame && piece instanceof King && piece.castled) ? 20 : 0
        //         //We might want to include how many pieces are protected by other pieces
        //         //Include mobility (how far a piece can travel)
        //         //Pawn promotion (moving the pawns to the end of the board)
        //         //Maybe start with some opening moves
        //         if (piece.color) 
        //         {
        //             assessment += points
        //             assessment += center
        //             assessment += developed
        //             assessment += castled
        //             assessment += push_pawns
        //         }
        //         else 
        //         {
        //             assessment -= points
        //             assessment -= center
        //             assessment -= developed
        //             assessment -= castled
        //             assessment -= push_pawns

        //         }
        //         if (isNaN(assessment)) {
        //             console.error("Assessment is not a number!", center, push_pawns, points, developed, castled)
        //         }
        //     }
        // }
        // // if (board.turn && board.valid_moves.size == 0)
        // // {
        // //     console.log("Checkmate found for black")
        // // } else if (!board.turn && board.valid_moves.size == 0)
        // // {
        // //     console.log("Checkmate found for white")
        // // }
        // return assessment/500
    // }


    staticEvaluation() 
    {
    

        //Count the pieces on the board
        let assessment = 0        
        
        //Assess by piece value
        for (var i=0; i<board.board[0].length; i++) 
        {
            for (var j=0; j<board.board.length; j++) 
            {
                let square = board.getSquare(i, j)
                if (!square.piece) continue
                let piece = square.piece

                //This just moves one piece to the center very often, instead of that lets go for number of pieces in the center
                let center = Math.sin(piece.x*Math.PI/8)*Math.sin(piece.y*Math.PI/8)*10
                
                //If in the endgame, try to keep the king in the center of the board
                //This also tries to move the opponents king to the edge of the board
                if (piece instanceof King)
                {
                    if (this.endgame) 
                    {
                        //Weight it very heavily as being towards the center
                        center *= 2
                    } else center = 0
                }

                //Try to push pawns to the end of the board
                if (piece instanceof Pawn) 
                {
                    if (this.endgame)
                    {
                        center = Math.abs(Math.cos(piece.y*Math.PI/7))*20
                    }
                }
            
            

                let push_pawns = 0

                let points = piece.points*100
                //Weight the queen as an even higher piece (to avoid sacrafices)
                if (piece instanceof Queen) points *= 2

                let developed = (!(piece instanceof Pawn) && !(piece instanceof Rook) && piece.moves < 1) ? -5 : 0
                let castled = (!this.endgame && piece instanceof King && piece.castled) ? 20 : 0
                //We might want to include how many pieces are protected by other pieces
                //Include mobility (how far a piece can travel)
                //Pawn promotion (moving the pawns to the end of the board)
                //Maybe start with some opening moves
                if (piece.color) 
                {
                    assessment += points
                    assessment += center
                    assessment += developed
                    assessment += castled
                    assessment += push_pawns
                }
                else 
                {
                    assessment -= points
                    assessment -= center
                    assessment -= developed
                    assessment -= castled
                    assessment -= push_pawns

                }
                if (isNaN(assessment)) {
                    console.error("Assessment is not a number!", center, push_pawns, points, developed, castled)
                }
            }
        }
        return assessment
    }
    //We also need to define any terminal states
    navigateInto(node) {
        //This will apply an action and transform into a new state
        board.move(node.move, true)
        board.turn = !board.turn
        board.step(true)
        this.current = node
        
    }

    getLargestChild() {
        let highest = null
        for (var child of this.current.children)
        {
            child.refreshUCB1()
            if (this.current.maximize)
            {
                if (!highest ||  child.UCB1 > highest.UCB1 ) highest = child
            }
            else
            {
                if (!highest || child.UCB1 < highest.UCB1 ) highest = child

            }

        }
        return highest
    }
}