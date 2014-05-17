import os, sys, subprocess

HAVE_VALGRIND = False
TEST_PATH = 'tests/bin'

def init():
    global HAVE_VALGRIND
    p = subprocess.Popen(['valgrind', '--help'], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    out, err = p.communicate()

    if p.returncode == 0 and len(out) > 0:
        HAVE_VALGRIND = True

def main():
    global HAVE_VALGRIND

    tests_run = 0
    tests_succeeded = 0
    
    for name in os.listdir(TEST_PATH):
        path = os.path.join(TEST_PATH, name)
        if os.path.isfile(path) and os.access(path, os.X_OK):
            tests_run += 1
            succeeded = True
            print('Running Test: %s' % (path))
            
            """
            if HAVE_VALGRIND:
                p = subprocess.Popen([
                    'valgrind',
                    '--leak-check=full',
                    '--error-exitcode=1',
                    path])
                
                p.communicate()
                if p.returncode == 0:
                    tests_succeeded += 1
                else:
                    succeeded = False
            else:
            """
            
            p = subprocess.Popen([path])
            p.communicate()
            if p.returncode == 0:
                tests_succeeded += 1
            else:
                succeeded = False

            if succeeded:
                print('Test %s Passed' % (path))
            else:
                print('Test %s Failed' % (path))

    print('%s out of %s Tests Succeeded' % (tests_succeeded, tests_run))
    sys.exit(0)

if __name__ == '__main__':
    init()
    main()
