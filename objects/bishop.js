class Bishop extends Piece {
    constructor(x, y, color, width) {
        super(x, y, color, width)
        this.points = 5
        this.image = color?bishop_img:bbishop_img
        
    }

    getName() { return "Bishop" }



    calculateValidMoves() {
        return this.getAttackMovesDiagnal()
    }
}