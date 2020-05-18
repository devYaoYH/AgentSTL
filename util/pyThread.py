import threading
import subprocess
from queue import Queue

STDIN = 0
STDOUT = 1
STDERR = 2

# Thread Work - Blocking
def output_stream_to_queue(stream, queue):
    for line in iter(stream.readline, b''):
        queue.put(line)

# Starts a process and writes output streams to queues for reading
class ProcQueue(threading.Thread):
    def __init__(self, proc_cmd, stdout_queue, stderr_queue=None):
        self.proc = None            # Process handle
        self.exception = None       # Exception during execution
        self.cmd = proc_cmd.split() # Command string to run
        self.outq = stdout_queue    # Queue to write stdout to
        self.errq = stderr_queue    # Queue to write stderr to
        threading.Thread.__init__(self)

    # Process ran into exception
    def has_exception(self):
        return self.exception

    def run(self):
        self.exception = None
        try:
            if (self.errq is None): # Redirect stderr to stdout if no stderr queue specified
                self.proc = subprocess.Popen(self.cmd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, stdin=subprocess.PIPE)
            else:
                self.proc = subprocess.Popen(self.cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, stdin=subprocess.PIPE)
        except Exception as e:
            print(e, file=sys.stderr)
            self.exception = e
            return

        # Start threads for writing to queues
        threads = []
        if (self.errq is None):
            # Start stdout thread ONLY
            stdout_proc = threading.Thread(target=output_stream_to_queue, args=[self.proc.stdout, self.outq])
            threads = [stdout_proc]
        else:
            stdout_proc = threading.Thread(target=output_stream_to_queue, args=[self.proc.stdout, self.outq])
            stderr_proc = threading.Thread(target=output_stream_to_queue, args=[self.proc.stderr, self.errq])
            threads = [stdout_proc, stderr_proc]

        for t in threads:
            t.start()

        # Wait for process to end
        for t in threads:
            t.join()

# Provides queues as interface for I/O to underlying process
# ready()   :   Returns true when we have thread started
# running() :   Returns true when thread is running
class IOProc(object):
    def __init__(self, cmd):
        self.cmd = cmd
        self.pipes = [Queue(), Queue(), Queue()]    # Stdin, Stdout, Stderr
        self.procThread = None

    # Wait for child
    def __del__(self):
        self.wait_child()

    # Dump pipes
    def wait_child(self):
        if (self.procThread is not None):
            # Blocking on main thread
            self.procThread.join()
            stdout_lines = self.read_pipe(1)
            stderr_lines = self.read_pipe(2)
            for l in stdout_lines:
                print(l.decode("ascii"),end="")
            for l in stderr_lines:
                print(l.decode("ascii"),end="")
            self.procThread = None
            print(f"[Thread: {self.cmd}] exited")

    # Initialize thread with option for new command
    def init(self, cmd=None):
        if (self.procThread is not None):
            if (self.procThread.poll() is None):
                return False
            else: # underlying process has finished
                self.wait_child()
        # We should have self.procThread as None here
        assert(self.procThread is None)
        if (cmd is not None):
            self.cmd = cmd
        self.procThread = ProcQueue(self.cmd, self.pipes[1], self.pipes[2])
        self.procThread.start()

    def ready(self):
        return self.procThread is not None and self.procThread.exception is None

    def running(self):
        return self.procThread is not None and self.procThread.exception is None and self.procThread.proc is not None and self.procThread.proc.poll() is None

    def has_bytes(self, fd):
        return not self.pipes[fd].empty()

    def read_pipe(self, fd):
        if (fd != 1 and fd != 2):
            return None
        lines = []
        while (not self.pipes[fd].empty()):
            lines.append(self.pipes[fd].get())
        return lines

    # Data is a bytes-like object
    def write(self, data):
        try:
            self.procThread.proc.stdin.write(data)
            self.procThread.proc.stdin.flush()
            return True
        except Exception as e:
            print(e, file=sys.stderr)
            return False