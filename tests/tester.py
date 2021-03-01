import subprocess as subpr
import os
from pathlib import Path

testpath = Path(__file__).parent
c4path = testpath.parent

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
                print(i, 'unexpected FAIL')
        else:
            if ccres.returncode:
                print(i, 'unexpected PASS')
