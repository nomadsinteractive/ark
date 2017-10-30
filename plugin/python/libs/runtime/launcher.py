import sys
from subprocess import Popen, PIPE, STDOUT, TimeoutExpired

if __name__ == '__main__':
    proc = Popen(sys.argv[1:], stdout=PIPE, stderr=STDOUT)
    while True:
        try:
            for line in iter(proc.stdout.readline, b''):
                print(line.decode(sys.stdout.encoding or 'utf-8').rstrip())
            proc.wait(timeout=1)
            break
        except TimeoutExpired:
            pass
