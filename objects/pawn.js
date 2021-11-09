class Pawn extends Piece {
    constructor(x, y, color, width) {
        super(x, y, color, width)
        this.points = 1
        this.image = color?pawn_img:bpawn_img
        this.forward = color? -1 : 1
        this.forward = board.flip? this.forward : -this.forward
    }

    getName() { return "Pawn" }



    calculateValidMoves() {
        let moves = []
        let current_square = board.getSquare(this.x, this.y)
        let square = null
        //Check one square up
        if ((square = board.getSquare(this.x, this.y - 1*this.forward)) != null && !square.piece) {
            moves.push(new Move(current_square, square))
        }

        //If in starting positions
        if ((this.forward == 1 && this.y == 6) || (this.forward == -1 && this.y == 1)) {
            if (!square.piece && (square = board.getSquare(this.x, this.y - 2*this.forward)) != null && !square.piece) {
                moves.push(new Move(current_square, square))
            }
        }

        //Check attacks to left
        if ((square = board.getSquare(this.x - 1, this.y - 1*this.forward)) != null && square.piece) {
            moves.push(new Move(current_square, square))
        }

        //Check attacks to right
        if ((square = board.getSquare(this.x + 1, this.y - 1*this.forward)) != null && square.piece) {
            moves.push(new Move(current_square, square))
        }


        //Check en passants
        if ((this.forward == 1 && this.y == 3) || (this.forward == -1 && this.y == 4)) {
            // let forward = (this.color && board.flip) ? 1 : -1
            let players_previous_move = board.moves[board.moves.length - 1]
            //Check if this last move was a pawn push and if it was in the right spot for an en passant
            if (players_previous_move && players_previous_move.destination.piece instanceof Pawn && players_previous_move.source.y == players_previous_move.destination.y-2*this.forward) {
                if (players_previous_move.destination.y == this.y) {
                    if (players_previous_move.destination.x == this.x-1) {
                        moves.push(new EnPassantMove(board.getSquare(this.x, this.y), board.getSquare(players_previous_move.destination.x, this.y-1*this.forward)))
                    } else if (players_previous_move.destination.x == this.x+1) {
                        moves.push(new EnPassantMove(board.getSquare(this.x, this.y), board.getSquare(players_previous_move.destination.x, this.y-1*this.forward)))
                    }
                }
            }
        }

        return moves
    }
}