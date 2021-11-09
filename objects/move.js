class Move {
    constructor(source, destination, takes_piece=null, is_castle=false, is_pawn_push=false) {
        this.source = source
        this.destination = destination
        this.is_castle = is_castle
        this.is_pawn_push = is_pawn_push
        this.takes_piece = takes_piece
        this.score = null
    }
}

class CastleMove extends Move {
    constructor(k_source, k_dest, r_source, r_dest) {
        super(k_source, k_dest)
        this.castle_move = new Move(r_source, r_dest, false, true)
    }
}

class EnPassantMove extends Move {
    constructor(source, destination) {
        super(source, destination)
    }
}