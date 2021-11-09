class Knight extends Piece {

    constructor(x, y, color, width) {
        super(x, y, color, width)
        this.points = 5
        this.image = color?knight_img:bknight_img
    }
    
    getName() { return "Knight" }


    calculateValidMoves() {
        let result = []
        let current_square = board.getSquare(this.x, this.y)
        let positions = [
            {x:this.x - 1, y:this.y + 2},
            {x:this.x - 1, y:this.y - 2},
            {x:this.x + 1, y:this.y + 2},
            {x:this.x + 1, y:this.y - 2},
            {x:this.x - 2, y:this.y + 1},
            {x:this.x - 2, y:this.y - 1},
            {x:this.x + 2, y:this.y + 1},
            {x:this.x + 2, y:this.y - 1},
        ]

        for (var position of positions) {
            let square = board.getSquare(position.x, position.y)
            if (!square) continue
            result.push(new Move(current_square, square))
        }
        return result
    }
}