import subprocess as subpr
import os
from pathlib import Path
from pprint import pprint

class bcolors:
    HEADER = '\033[95m'
    OKBLUE = '\033[94m'
    OKCYAN = '\033[96m'
    OKGREEN = '\033[92m'
    WARNING = '\033[93m'
    FAIL = '\033[91m'
    ENDC = '\033[0m'
    BOLD = '\033[1m'
    UNDERLINE = '\033[4m'

output_cerr = 0
    
testpath = Path(__file__).parent
c4path = testpath.parent

unxpass = []
unxfail = []

for i in os.listdir(testpath):
    if '.c' in i:
        res = subpr.run([
            c4path / './build/debug/c4',
            '--print-ast',
            testpath / i
        ], capture_output = True)

        ccres = subpr.run([
            'cc',
            '-fsyntax-only',
            '--pedantic',
            testpath / i
        ], capture_output = True)

        if res.returncode:
            if not ccres.returncode:
                unxfail.append(i)
        else:
            if ccres.returncode:
                unxpass.append(i)
                
if __name__ == '__main__':
    print(f'Unexpected {bcolors.FAIL}FAIL{bcolors.ENDC}:')
    pprint(unxfail)
    print()

    
    print(f'Unexpected {bcolors.OKCYAN}PASS{bcolors.ENDC}:')
    pprint(unxpass)
    print()

    if output_cerr:
        print(f'{bcolors.HEADER}{bcolors.BOLD}cc errors{bcolors.ENDC}:')
        for i in unxpass:
            ccres = subpr.run([
                'cc',
                '-fsyntax-only',
                '--pedantic',
                testpath / i
            ], capture_output = False)
