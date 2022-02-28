// let FEN = "r1b1k1nr/p2p1pNp/n2B4/1p1NP2P/6P1/3P1Q2/P1P1K3/q5b1" //Starting pos 1
// let FEN = '1nb1k1nr/1p3p1p/2p3p1/8/1b1pPK2/N2P2N1/P1P1BPq1/R4R2'
// let FEN = '8/P7/8/8/8/8/p7/8'
// let FEN = 'K7/8/8/7k8/8/3r4/1r6'
// let FEN = 'r6k/8/8/8/8/K7/8/r7'
let FEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR" //Regular starting pos
let flip = 0
// let FEN = '8/8/8/8/4p3/8/5P2/8' //En Passant test 1
// let FEN = '8/4p3/8/8/3P1P2/8/1p6/8' //En Passant test 2
// let FEN = "3r2KR/r7/8/8/8/8/k7/7R" //Checkmate 1
// let FEN = '3r2KR/r7/8/8/8/8/k7/7R'
// let FEN = "8/P7/8/8/K6k/8/p7/8"
//white pieces
let pawn_img
let rook_img
let queen_img
let bishop_img
let knight_img
let king_img

//black pieces
let bpawn_img
let brook_img
let bqueen_img
let bknight_img
let bking_img
let bbishop_img

let piece_move_sound
let attack_sound
let checkmate_sound
let check_sound

function preload() {
    pawn_img = loadImage('assets/pawn.png');
    rook_img = loadImage('assets/rook.png')
    queen_img = loadImage('assets/queen.png')
    bishop_img = loadImage('assets/bishop.png')
    king_img = loadImage('assets/king.png')
    knight_img = loadImage('assets/knight.png')

    bpawn_img = loadImage('assets/bpawn.png');
    brook_img = loadImage('assets/brook.png')
    bqueen_img = loadImage('assets/bqueen.png')
    bbishop_img = loadImage('assets/bbishop.png')
    bknight_img = loadImage('assets/bknight.png')
    bking_img = loadImage('assets/bking.png')

    //Load in our sounds
    piece_move_sound = loadSound('assets/piece_move.mp3');
    attack_sound = loadSound('assets/attack_sound.mp3')
    checkmate_sound = loadSound('assets/gameover.mp3')
    check_sound = loadSound('assets/check.mp3')
}   


function setup() {
    let width = screen.height*.7
    createCanvas(width, width);
    initGame()
}

function initGame()
{
    let width = screen.height*.7
    board = new Board(width)
    board.engine = new Engine(3)
    board.engine2 = new MonteCarlo(3200)
    board.initialize()
}

function draw() {
    noStroke();
    board.display()
}

function mousePressed(event){
    board.mousePressed(event)
}

function keyPressed() {
    if (keyCode === LEFT_ARROW) {
        if (board.undo()) {
            board.turn = !board.turn
            board.step()
        }
    } else if (keyCode === 32)
    {
        flip = !flip
        initGame()
    }

}
