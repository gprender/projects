# grep.py - Graeme's (other) Regular Expression Parser
#
# I know a few more tricks now than I did a year ago when I first
# attempted to write a regular expression parser from scratch,
# so let's give this another shot.
#
# My goal is to use a recursive object-oriented approach:
# we'll have a top-level expression, which is evaluated by recursively
# evaluating all sub-expressions with a given input string.


class Expression:
    '''A parent expression, comprised of 1 or more sub-expressions.'''
    def __init__(self, pattern):
        # should check that the pattern isn't empty
        self.subexpressions = []
        for char in pattern:
            self.subexpressions.append(Primitive(char))

    ''' TODO
    def __str__(self):
        print("[ ", end="")
        for expr in self.subexpressions:
            print(f"{expr}, ", end="")
        print("]")

        return f"{}" '''

    
    def eval(self, string):
        remaining = string
        for expr in self.subexpressions:
            if remaining == '':
                return False
            else:
                remaining = expr.eval(remaining)
                if remaining == False:
                    return False

        return True


class Primitive:
    '''A single (non-special) character in a pattern.'''
    def __init__(self, char):
        self.char = char

    def __str__(self):
        return (f"Primitive: {self.char}")

    def eval(self, string):
        if string[0] == self.char:
            return string[1:]
        else:
            return False


def main():
    foo = Expression("abba")
    
    print(foo.eval("faust"))
    print(foo.eval("abba"))
    print(foo.eval("abbab")) # Unintended accept - why?


if __name__ == '__main__':
    main()