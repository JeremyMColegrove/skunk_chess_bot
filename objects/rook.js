class Rook extends Piece {
    constructor(x, y, color, width) {
        super(x, y, color, width)
        this.points = 6
        if (color == 1)
            this.image = rook_img
        else 
            this.image = brook_img
    }

    getName() { return "Rook" }

    
    calculateValidMoves() {
        return this.getAttackMovesStraight()
    }
}