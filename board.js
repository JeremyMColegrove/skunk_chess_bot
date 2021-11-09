
class Board {
    constructor(width) {
        this.flip = flip
        this.width = width - (width%8)
        //whos turn it is (1 for white, 0 for black)
        this.board = null
        //squares that are under attack by the other color
        this.under_attack_tiles = new Map()
        //Moves that are valid to perform for each piece
        this.valid_moves = new Map()
        //Tiles that are valid for pieces to land on
        this.valid_tiles = new Map()
        //Squares that light up when a piece is clicked
        this.illuminated_squares = new Map()
        //Load in a computer engine to play
        this.engine = null
        this.engine2 = null
        //Representing whos turn it is (1 = white, 0 = black)
        // this.turn = this.flip?0:1
        this.turn = 1

        //The currently selected piece
        this.selected = null

        //list move moves that have already happened
        this.moves = []

        // array of all of the pieces
        this.pieces = []

    }

    initialize() {
        this.constructBoard()
        this.step()
        if (this.engine2)
        {
            this.dual()
        }
    }

    dual()
    {
        setTimeout(()=>
        {
            this.engine.playMove(100)
            this.engine2.playMove(100)
            this.dual()
        }, 100)
        
    }
    //Construct the board based off of a FEN string
    constructBoard() {
        this.tile_width = this.width/8;

        this.board = new Array(8).fill().map(x=>new Array(8).fill(0))
        var y_rank = this.flip? 7 : 0
        var x_rank = this.flip? 7 : 0
        this.turns = 0
        for (var i=0; i<this.board[0].length; i++) {
            for (var j=0; j<this.board.length; j++) {
                this.board[j][i] = new Tile(i, j, this.tile_width, null)
            }
        }


        for (var i=0; i<FEN.length; i++) {
            let char = FEN.charAt(i)
            let tile = this.board[y_rank][x_rank]
            switch(char) {
                case '/':
                    this.flip? y_rank-- : y_rank++
                    this.flip? x_rank=8 : x_rank=-1
                    break
                case 'r':
                    tile.piece = new Rook(tile.x, tile.y, 0, this.tile_width)
                    break
                case 'R':
                    tile.piece = new Rook(tile.x, tile.y, 1, this.tile_width)
                    break
                case 'p':
                    tile.piece = new Pawn(tile.x, tile.y, 0, this.tile_width)
                    break
                case 'P':
                    tile.piece = new Pawn(tile.x, tile.y, 1, this.tile_width)
                    break
                case 'q':
                    tile.piece = new Queen(tile.x, tile.y, 0, this.tile_width)
                    break
                case 'Q':
                    tile.piece = new Queen(tile.x, tile.y, 1, this.tile_width)
                    break
                case 'k':
                    tile.piece = new King(tile.x, tile.y, 0, this.tile_width)
                    break
                case 'K':
                    tile.piece = new King(tile.x, tile.y, 1, this.tile_width)
                    break
                case 'b':
                    tile.piece = new Bishop(tile.x, tile.y, 0, this.tile_width)
                    break
                case 'B':
                    tile.piece = new Bishop(tile.x, tile.y, 1, this.tile_width)
                    break
                case 'r':
                    tile.piece = new Rook(tile.x, tile.y, 0, this.tile_width)
                    break
                case 'R':
                    tile.piece = new Rook(tile.x, tile.y, 1, this.tile_width)
                    break
                case 'n':
                    tile.piece = new Knight(tile.x, tile.y, 0, this.tile_width)
                    break
                case 'N':
                    tile.piece = new Knight(tile.x, tile.y, 1, this.tile_width)
                    break
                default:
                    let space = parseInt(char)
                    if (space != NaN) {
                        for (var j=0; j<space-1; j++) x_rank++
                    } else {
                        console.log("Bad FEN string. Unknown character ", char)
                    }
            }     
            if (tile && tile.piece)
                this.pieces.push(tile.piece)       
            this.flip? x_rank-- : x_rank++
        }
    }

    getFEN() {
        let fen = ''
        let space = 0
        for (var i=0; i<this.board[0].length; i++) 
        {
            for (var j=0; j<this.board.length; j++)
            {
                let square = this.getSquare(i, j)
                if (square.piece)
                {
                    if (space > 0) fen += space
                    space = 0
                    //Insert correct piece here
                    let letter = ''
                    let piece = square.piece
                    if (piece instanceof Pawn) letter = 'p'
                    if (piece instanceof Rook) letter = 'r'
                    if (piece instanceof King) letter = 'k'
                    if (piece instanceof Queen) letter = 'q'
                    if (piece instanceof Bishop) letter = 'b'
                    if (piece instanceof Knight) letter = 'n'
                    if (piece.color) letter = letter.toUpperCase()
                    fen += letter
                } else 
                {
                    space ++
                }
            }
            if (space > 0) fen += space
            space = 0
            fen += '/'
        }
        return fen
    }

    
    step(mute=false) {
        //Calculate all of the valid moves for this turn
        this.valid_moves.clear()
        this.valid_tiles.clear()
        //Calculate it in the beginning so the king knows if he is able to castle or not
        this.calculateUnderAttackSquares()
        for (var i=0; i<this.board[0].length; i++) {
            for (var j=0; j<this.board.length; j++) {
                let square = this.getSquare(i, j)
                if (square.piece && square.piece.color == this.turn) {
                    //Get all of the moves on the board
                    let pieces_moves = square.piece.calculateValidMoves()
                    //Now strip away the ones from the other player, and filter ours by the ones that dont cause check
                    pieces_moves = pieces_moves.filter(move=> {
                        if (move.destination.piece && (move.destination.piece.color == square.piece.color && square.piece != move.destination.piece)) return false

                        let valid_move = true
                        this.move(move, true)
                        this.calculateUnderAttackSquares()
                        for (var piece of this.pieces) {
                            //If this move puts us in check, throw it out
                            if (piece instanceof King && piece.color == this.turn && piece.inCheck()) valid_move = false
                        }
                        this.undo()
                        this.calculateUnderAttackSquares()
                        
                        return valid_move
                    })

                    //Keep track of valid moves for the future (bot engines)
                    this.valid_moves.set(square.piece, pieces_moves)
                    //Keep track of valid tiles to land on for the future (checks, etc)
                    for (var move of pieces_moves) {
                        this.valid_tiles.set(move.destination, move.source.piece)
                    }
                }
            }
        }

        //Check if the current person is in checkmate (uh oh!)
        for (var entry of this.valid_moves) {
            if (entry[0] instanceof King && entry[0].color == this.turn) {
                // let checkmate = entry[0].inCheckmate()
                                
                if (entry[1].length == 0 && entry[0].inCheckmate()) {
                    if (!mute) checkmate_sound.play()
                    // console.log("Checkmate!")
                    // if (this.turn == 0) console.log("White wins!")
                    // else console.log("Black wins!")
                } else if (!mute && entry[0].inCheck()) {
                    check_sound.play()
                }
            }
        }
    }

    calculateUnderAttackSquares(color=this.turn) {
        //Must get the under attack squares by the other color (good for checking if in checkmate and such)
        this.under_attack_tiles.clear()
        for (var i=0; i<this.board[0].length; i++) {
            for (var j=0; j<this.board.length; j++) {
                let square = this.getSquare(i, j)
                if (square.piece && square.piece.color != this.turn) {
                    let pieces_moves = square.piece.calculateValidMoves()
                    //Filter the ones from the other player from the ones from us
                    pieces_moves.forEach(move=>{
                        if (move.source.piece && move.source.piece.color != color) {
                            if (move.source.piece instanceof Pawn) {
                                //Exception here -- we don't want to add its movement
                                let forward = color == 0 ? 1 : -1
                                this.under_attack_tiles.set(this.getSquare(move.source.x-1, move.source.y - forward), move.source.piece)
                                this.under_attack_tiles.set(this.getSquare(move.source.x+1, move.source.y - forward), move.source.piece)
                            } else {
                                this.under_attack_tiles.set(move.destination, move.source.piece)
                            }
                            //Add this move to the under attack squares
                        }
                    })
                }
            }
        }
    }

    display() {
        for (var x=0; x<this.board[0].length; x++) {
            for (var y=0; y<this.board.length; y++) {
                let tile = this.getSquare(x, y)
                tile.display()
                if (tile.piece != null) {
                    tile.piece.display()
                }
            }
        }
    }

    mousePressed(event) {
        let xt = Math.floor(event.offsetX/this.tile_width)
        let yt = Math.floor(event.offsetY/this.tile_width)
        let tile = this.board[yt][xt]
        //If we are moving a piece (rather than selecting another)
        // let is_attack = attack_moves.has(square.tile)
        if (this.selected && this.selected.piece != null && (this.illuminated_squares.has(tile))) {
            if (this.selected.piece == tile.piece) {
                this.selected = null
            } else {
                //Don't make a new move, but use the move from the piece that is selected
                let pieces_moves = this.valid_moves.get(this.selected.piece)
                let move = pieces_moves.filter(move=>move.destination==tile)[0]
                //If the valid tile that was clicked is not in fact part of the pieces moves, just deselect the piece
                if (!move) {
                    this.selected = null
                } else {
                    this.move(move)
                    //Toggle the turn
                    this.turn=!this.turn
                    this.step()

                    if (this.engine) {
                        this.engine.playMove(100)
                    }
                }
            }
        } else  {
            this.selected = tile
            this.illuminated_squares.clear()
            let available_squares = this.valid_moves.get(this.selected.piece)
            
            if (available_squares)
                available_squares.forEach(move=> {
                    this.illuminated_squares.set(move.destination, 1)
                })
        }
    }

 

    getSquare(x, y) {
        if (x < 0 || x > 7 || y < 0 || y > 7) {
            return null
        }
        return this.board[y][x]
    }

    move(move, mute=false) {
        if (!move.source.piece) {
            console.error("Tried to pass a move", move, "which is not valid because it does not contain a piece.")
            return
        }


        move.takes_piece = move.destination.piece
        move.destination.piece = move.source.piece
        move.destination.piece.x = move.destination.x
        move.destination.piece.y = move.destination.y
        move.destination.piece.moves ++
        this.turns ++
        //If it was a pawn push
        if (move.destination.piece instanceof Pawn && (move.destination.y == 0 || move.destination.y == 7)) {
            move.is_pawn_push = true
            let queen = new Queen(move.destination.piece.x, move.destination.piece.y, move.destination.piece.color, move.destination.piece.width)
            move.destination.piece = null
            move.destination.piece = queen
        }
        
        if (move instanceof EnPassantMove) {
            let forward = move.destination.piece.color == 1 ? 1 : -1
            let taking_tile = this.getSquare(move.destination.x, move.destination.y+1*forward)
            taking_tile.piece = null
        }
        
        move.source.piece = null

        this.moves.push(move)
        this.selected = null
        this.illuminated_squares.clear()

        //If it was a castle, move the rook now
        if (move instanceof CastleMove) {
            move.destination.piece.castled = true
            this.move(move.castle_move, mute)
        }

        //Figure out which sound to play
        if (!mute) {
            // let fen = document.getElementById('fen')
            // fen.innerHTML = this.getFEN()
            if (move.takes_piece) {
                attack_sound.play()
            } else {
                piece_move_sound.play()
            }
        }
        
    }

    undo() {
        //Reverse the previous move (go back in time)
        if (this.moves.length == 0) return false

        let move = this.moves.pop()
        move.source.piece = move.destination.piece
        move.destination.piece = move.takes_piece
        move.source.piece.x = move.source.x
        move.source.piece.y = move.source.y
        move.source.piece.moves --
        this.turns --


        if (move.is_pawn_push) {
            let pawn = new Pawn(move.source.x, move.source.y, move.source.piece.color, move.source.piece.width)
            move.source.piece = null
            move.source.piece = pawn
        }

        if (move.is_castle) {
            this.undo()
        }
        if (move instanceof CastleMove) {
            move.source.piece.castled = false

        }

        if (move instanceof EnPassantMove) {
            let forward = move.source.piece.color == 1 ? 1 : -1
            let tile = this.getSquare(move.destination.x, move.destination.y+1*forward)
            tile.piece = new Pawn(tile.x, tile.y, !move.source.piece.color, tile.width)
        }

        this.selected = null
        this.illuminated_squares.clear()
        return true
    }

    getPieceCount() 
    {
        let pieces = [0, 0]
        for (var i=0; i<board.board[0].length; i++) 
        {
            for (var j=0; j<board.board.length; j++) 
            {
                let square = board.getSquare(i, j)
                if (square.piece) 
                {
                    pieces[square.piece.color]++
                } 
            }
        }
        return pieces
    }
}