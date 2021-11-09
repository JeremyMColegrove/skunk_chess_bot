class Tile {
    constructor(x, y, width, piece) {
        this.x = x
        this.y = y
        this.width = width
        this.piece = piece
    }

    display() {
        let green = (this.x%2 + this.y%2-1 == 0)
        if (green) {
            // fill(124, 149, 92)
            fill(124, 149, 172)

        } else {
            fill(238, 238, 213)
        }

        if (board.selected && board.selected == this && board.selected.piece != null) {
            if (green)
                fill(190, 202, 89)
            else
                fill(247, 247, 149)
        }

        square(this.width*this.x, this.width*this.y, this.width)
        if (board.illuminated_squares.has(this)) {

            let c = color(20, 20, 20)
            c.setAlpha(25)
            fill(c)
            if (this.piece)
            circle(this.x*this.width+this.width/2, this.y*this.width+this.width/2, this.width)
            else circle(this.x*this.width+this.width/2, this.y*this.width+this.width/2, 25)
        }
    }
}