class Queen extends Piece {
    constructor(x, y, color, width) {
        super(x, y, color, width)
        this.points = 8
        this.image = color?queen_img:bqueen_img
    }

    getName() { return "Queen" }


    calculateValidMoves() {
        return [...this.getAttackMovesDiagnal(), ...this.getAttackMovesStraight()]
    }
}