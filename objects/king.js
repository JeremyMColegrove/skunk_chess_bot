class King extends Piece {
    constructor(x, y, color, width) {
        super(x, y, color, width)
        this.points = 100
        this.castled = false
        this.image = color?king_img:bking_img
        this.forward = board.color? -1 : 1
        this.forward = board.flip? -this.forward : this.forward
        // this.refreshColor()
        
    }

    getName() { return "King" }

    inCheck() {
        if (board.under_attack_tiles.has(board.getSquare(this.x, this.y))) {
            return true
        }
        return false
    }

    inCheckmate() {
        //Make each move and check if this piece is still in checkmate
        let checkmate = true
        for (var piece of board.valid_moves) {
            for (var _move of piece[1]) {
                board.move(_move, true)
                board.calculateUnderAttackSquares()
                checkmate = this.inCheck()
                board.undo()
                board.calculateUnderAttackSquares()
                if (!checkmate) break
            }
            if (!checkmate) break
        }
        return checkmate
    }

    calculateValidMoves() {
        let moves = []
        let current_square = board.getSquare(this.x, this.y)
        for (var i=0; i<3; i++) {
            for (var j=0; j<3; j++) {
                if (i == 1 && j == 1) continue
                var ii = this.x + i - 1;
                var jj = this.y + j - 1;
                let square = board.getSquare(ii, jj)
                if (!square) continue
                moves.push(new Move(current_square, square))
            }
        }

        
        if (this.moves > 0) return moves

        //Check for castle move to the right
        let right_castle = true
        let left_castle = true

        for (var i=this.x+1; i < 7; i++) {
            let tile = board.getSquare(i, this.y)
            if (tile.piece || board.under_attack_tiles.has(tile) || this.inCheck()) {
                right_castle = false
                break
            }
        }

        for (var i=this.x-1; i >= 1; i--) {
            let tile = board.getSquare(i, this.y)
            if (tile.piece || (board.under_attack_tiles.has(tile)&&i!=1) || this.inCheck()) {
                left_castle = false
                break
            }
        }


        if (right_castle) {
            let tile = board.getSquare(7, this.y)
            if (tile.piece && tile.piece instanceof Rook && tile.piece.moves == 0) {
                //This is a valid castle
                let k_source = board.getSquare(this.x, this.y)
                let k_dest = board.getSquare(this.x+2, this.y)
                let r_source = board.getSquare(7, this.y)
                let r_dest = board.getSquare(this.x+1, this.y)
                moves.push(new CastleMove(k_source, k_dest, r_source, r_dest))
            }
        }
        
        

        if (left_castle) {
            let tile = board.getSquare(0, this.y)
            if (tile.piece && tile.piece instanceof Rook && tile.piece.moves == 0) {
                //This is a valid castle
                let k_source = board.getSquare(this.x, this.y)
                let k_dest = board.getSquare(this.x-2, this.y)
                let r_source = board.getSquare(0, this.y)
                let r_dest = board.getSquare(this.x-1, this.y)
                moves.push(new CastleMove(k_source, k_dest, r_source, r_dest))
            }
        }
        
        
        return moves
    }
}