import sys
import time
from util.pyThread import STDIN,STDOUT,STDERR,IOProc

# AI_test = './bin/npuzzle_test -p astar -w 10 -s 100000 -n 5'
AI_test = './bin/sokoban_test -p astar -w 10'

engine = IOProc(AI_test)
engine.init()

# Block and wait for interactive subprocess init
while (not engine.ready()):
    #spin
    time.sleep(0.001)

EXIT_CODE = None

def do_stuff():
    global EXIT_CODE
    print(f">>>",file=sys.stderr,end="")
    #We're waiting on input
    in_line = input() + "\n"
    engine.write(in_line.encode())
    if (in_line.strip() == "EXIT"):
        EXIT_CODE = True
    time.sleep(0.1)

# Give a bit of startup time
time.sleep(0.2)

# Interactive output?
while (not EXIT_CODE and engine.running()):
    if (engine.has_bytes(STDOUT)):
        lines = engine.read_pipe(STDOUT)
        for line in lines:
            print(line.decode("ascii"),end="")
        sys.stdout.flush()
    else:
        do_stuff()

engine.wait_child()