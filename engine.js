class Engine {
    constructor(depth) {
        this.depth = depth
        this.move = null
        this.endgame = false
    }
    
    
    playMove(pause=null) 
    {
        if (pause) 
        {
            setTimeout(()=>this.playMove(), pause)
            return
        }

        //Maybe increase our depth by 1 in the endgame since there is significantly less pieces
        let pieces = board.getPieceCount()

        if (!this.endgame && pieces[0] + pieces[1] < 20) 
        {
            console.warn("Engine is switching to endgame strategy")
            // if (this.depth < 3) this.depth ++
            this.endgame = true
        }


        let move = this.alphabeta(this.depth, -Infinity, Infinity, !board.turn)
        if (!move.best) {
            console.warn("Engine tried to make an invalid move to", move, `(typically means you have checkmate in ${this.depth} moves and it doesn't have a way to stop it)`)
            return
        }

        //We could do one ply search to figure out the relatively best moves, and then search those moves accordingly
        
        //What is the engine thinking?
        let thoughts = ''
        if (move.score < 0) thoughts = `The engine thinks it is winning by ${(move.score/1000).toFixed(2)}`
        else thoughts = `The engine thinks you are winning by ${(move.score/1000).toFixed(2)}`
        document.getElementById('fen').innerHTML = thoughts


        board.move(move.best)
        board.turn = !board.turn
        board.step()
    }


    alphabeta(depth, alpha, beta, maximizingPlayer) 
    
    {
        //We might want to keep collecting pieces if there are captures available and assess that score
        if (depth == 0) 
        {
            return {best:null, score:this.staticEvaluation()}
        }

        //https://en.wikipedia.org/wiki/Alpha–beta_pruning
        if (!maximizingPlayer) 
        {
            let value = {best:null, score:-Infinity}
            let moves = Array.from(board.valid_moves, ([name, value]) => value).flat();

            for (var move of moves) 
            {
                
                let result = this.movePiece(move, depth, alpha, beta, maximizingPlayer)

                if (result.score > value.score && move) 
                {
                    value = result
                    value.best = move
                }

                if (result.score >= beta) 
                {
                    break
                }
                alpha = Math.max(result.score, alpha)
            }
            return value
        } else 
        {
            let value = {best:null, score:Infinity}
            let moves = Array.from(board.valid_moves, ([name, value]) => value).flat();

            for (var move of moves) 
            {
                let result = this.movePiece(move, depth, alpha, beta, maximizingPlayer)

                if (result.score < value.score && move)
                {
                    value = result
                    value.best = move
                }

                if (result.score <= alpha) 
                {
                    break
                }
                beta = Math.min(result.score, beta)
            }
            return value
        }
    }

    movePiece(move, depth, alpha, beta, maximizingPlayer) 
    {
        board.move(move, true)
        board.turn = !board.turn
        board.step(true)

        let value = this.alphabeta(depth-1, alpha, beta, !maximizingPlayer)


        board.undo()
        board.turn = !board.turn
        board.step(true)

        return value
    }

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
}