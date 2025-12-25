import subprocess
import time
import re

def run_engine_test():
    process = subprocess.Popen(
        ['./Skunk'],
        stdin=subprocess.PIPE,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True
    )

    commands = [
        "uci",
        "isready",
        "position startpos",
        "go depth 8"
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
        
        # Timeout after 30 seconds
        if time.time() - start_time > 30: 
            print("Timeout!")
            process.kill()
            break

    process.terminate()

if __name__ == "__main__":
    run_engine_test()
