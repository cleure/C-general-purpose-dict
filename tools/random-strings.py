import os, sys

class EntropyGenerator(object):
    def __init__(self, source='/dev/urandom'):
        self.fp = open(source, 'rb')

    def get_entropy(self, length):
        data = self.fp.read(length)
        string = ''
        
        for byte in data:
            string += chr((ord(byte) % 26) + 65)

        return string

def main(iterations, str_length):
    generator = EntropyGenerator()
    items = set()
    
    for i in range(iterations):
        length = len(items)
        item = None
        while len(items) == length:
            item = '"' + generator.get_entropy(str_length) + '"'
            items.add(item)

        if (i + 1) == iterations:
            print('    %s' % (item))
        else:
            print('    %s,' % (item))

    sys.exit(0)

if __name__ == '__main__':
    iterations = 16
    str_length = 16
    
    if len(sys.argv) > 1:
        iterations = int(sys.argv[1])
    
    if len(sys.argv) > 2:
        str_length = int(sys.argv[2])

    main(iterations, str_length)
