import sys
import time
import threading
import subprocess
from queue import Queue, Empty

class EngineThread(threading.Thread):
    def __init__(self, output_queue):
        self.proc = None
        self.init = True
        self.outq = output_queue
        threading.Thread.__init__(self)

    def run(self):
        try:
            self.proc = subprocess.Popen('./bin/npuzzle_test -p astar -w 10 -s 100000 -n 5'.split(),
            # self.proc = subprocess.Popen('./bin/sokoban'.split(),
                stdout=subprocess.PIPE, stderr=subprocess.STDOUT, stdin=subprocess.PIPE)
        except Exception as e:
            print(e, file=sys.stderr)
            self.init = False
            return
        # Blocking bit
        for line in iter(self.proc.stdout.readline, b''):
            self.outq.put(line)
        # End of program execution?

proc_output = Queue()

engine = EngineThread(proc_output)
engine.start()

# Block and wait for interactive subprocess init
while (not engine.proc and engine.init):
    #spin
    time.sleep(0.001)

if (engine.init):
    print(f"Proc init: {engine.proc.poll()}")
else:
    print("Exception occured in subprocess init")
    exit(1)

def do_stuff():
    print(f">>>",file=sys.stderr,end="")
    #We're waiting on input
    in_line = input() + "\n"
    try:
        engine.proc.stdin.write(in_line.encode())
        engine.proc.stdin.flush()
    except Exception as e:
        print(e, file=sys.stderr)
    time.sleep(0.1)

# Give a bit of startup time
time.sleep(0.2)

# Interactive output?
while (engine.proc.poll() is None):
    try:
        line = proc_output.get_nowait()
        print(line.decode("ascii"),end="")
        sys.stdout.flush()
    except Empty:
        do_stuff()

while (not proc_output.empty()):
    line = proc_output.get()
    print(line.decode("ascii"),end="")
    sys.stdout.flush()

print("proc finished")

# Flush Rest of output [Not necessary?]
engine.join()
remainder = engine.proc.communicate()[0]
print(remainder.decode("ascii"))