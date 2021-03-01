import subprocess as subpr
import os

for i in os.listdir('./'):
    if '.c' in i:
        res = subpr.run([
            '../build/debug/c4',
            '--print-ast',
            i
        ], capture_output = True)

        ccres = subpr.run([
            'cc',
            '-fsyntax-only',
            '--pedantic',
            i
        ], capture_output = True)

        if res.returncode:
            if not ccres.returncode:
                print(i, 'unexpected pass')
        else:
            if ccres.returncode:
                print(i, 'unexpected fail')
