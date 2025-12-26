import subprocess
import time

def run_engine_test():
    process = subprocess.Popen(
        ['./Skunk'],
        stdin=subprocess.PIPE,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True
    )

    # Test the position where bishop sacrifice bug occurs
    # Black to move - should NOT sacrifice bishop for pawn
    commands = [
        "uci",
        "isready",
        "position fen r2qkb1r/ppp1pppp/2n2n2/3p1b2/3P1BP1/2N2N1P/PPP1PP2/R2QKB1R b KQkq - 0 6",
        "go depth 7"
    ]

    start_time = time.time()
    for cmd in commands:
        process.stdin.write(cmd + "\n")
    process.stdin.flush()

    output = ""
    while True:
        line = process.stdout.readline()
        if not line:
            break
        output += line
        print(line.strip())
        if "bestmove" in line:
            break
        
        # Timeout after 60 seconds
        if time.time() - start_time > 60: 
            print("Timeout!")
            process.kill()
            break

    process.terminate()

if __name__ == "__main__":
    run_engine_test()
