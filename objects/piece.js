class Piece {
    constructor(x, y, color, width) {
        this.color = color
        this.image = null
        this.width = width
        this.moves = 0
        this.x = x
        this.y = y
    }

    getAttackMovesStraight() {
        //Get all of the visible tiles
        //Search horizontal to the right
        let moves = []
        let current_tile = board.getSquare(this.x, this.y)
        for (var i=this.x+1; i<8; i++) {
            let square = board.getSquare(i, this.y)            
            moves.push(new Move(current_tile, square))
            if (square.piece) break
        }

        //Search horizontal to the left
        for (var i=this.x-1; i>=0; i--) {
            let square = board.getSquare(i, this.y)            
            moves.push(new Move(current_tile, square))
            if (square.piece) break
        }

        //Search vertical down
        for (var i=this.y+1; i<8; i++) {
            let square = board.getSquare(this.x, i)            
            moves.push(new Move(current_tile, square))
            if (square.piece) break
        }

        //Search vertical up
        for (var i=this.y-1; i>=0; i--) {
            let square = board.getSquare(this.x, i)            
            moves.push(new Move(current_tile, square))
            if (square.piece) break
        }
        return moves
    }

    getAttackMovesDiagnal() {
        let moves = []
        let current_tile = board.getSquare(this.x, this.y)
        //Search diagnal right down
        for (var i=this.x+1, j=this.y+1; i<8 && j<8; i++, j++) {
            let square = board.getSquare(i, j)            
            moves.push(new Move(current_tile, square))
            if (square.piece) break
        }

        //Search diagnal right up
        for (var i=this.x+1, j=this.y-1; i<8 && j>=0; i++, j--) {
            let square = board.getSquare(i, j)            
            moves.push(new Move(current_tile, square))
            if (square.piece) break
        }

        //Search diagnal left up
        for (var i=this.x-1, j=this.y-1; i>=0 && j>=0; i--, j--) {
            let square = board.getSquare(i, j)            
            moves.push(new Move(current_tile, square))
            if (square.piece) break
        }

        //Search diagnal left up
        for (var i=this.x-1, j=this.y+1; i>=0 && j<8; i--, j++) {
            let square = board.getSquare(i, j)            
            moves.push(new Move(current_tile, square))
            if (square.piece) break
        }
        return moves
    }
    
    display() {
        image(this.image, this.x*this.width, this.y*this.width, this.width, this.width)
    }
}
