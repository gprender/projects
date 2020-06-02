# grep.py - Graeme's (other) Regular Expression Parser
#
# I know a few more tricks now than I did a year ago when I first
# attempted to write a regular expression parser from scratch,
# so let's give this another shot.
#
# My overall goal is to use a recursive object-oriented approach:
# we'll have a top-level expression, which is evaluated by recursively
# evaluating all sub-expressions for the given input string.

'''
TODO List:
 - Parentheses
 - Union/Or (The '|' symbol)
'''

class Expression:
    '''A parent expression, comprised of 1 or more sub-expressions.'''
    def __init__(self, pattern):
        self.subexpressions = []
        for char in pattern:
            if char == '*': # Wrap the previous expression in a Star
                prev = self.subexpressions[-1]
                self.subexpressions[-1] = Star(prev)
            else:
                self.subexpressions.append(Primitive(char))

    def __str__(self):
        str_subexprs = [str(expr) for expr in self.subexpressions]
        return f"[ {', '.join(str_subexprs)} ]" 

    def eval(self, string): # There might be a cleaner way to do this?
        '''Recursively evaluate an expression for the given string.'''
        remaining = string
        for expr in self.subexpressions:
            if remaining == '':
                return False
            else:
                remaining = expr.eval(remaining)
                if remaining == False:
                    return False
        return remaining == ''


class Primitive:
    '''A single (non-special) character in a pattern.'''
    def __init__(self, char):
        self.char = char

    def __str__(self):
        return f"Primitive: {self.char}"

    def eval(self, string):
        '''A primitive matches itself and nothing else. Simple!'''
        if string[0] == self.char:
            return string[1:]
        else:
            return False


class Star:
    '''A Kleene Star expression, comprised of a single subexpression.'''
    def __init__(self, expr):
        self.subexpression = expr

    def __str__(self):
        return f"Star({self.subexpression})"

    def eval(self, string):
        '''Evalutate the subexpression 0 or more times with the given string.'''
        remaining = string
        next = self.subexpression.eval(remaining)
        while(next):
            remaining = next
            next = self.subexpression.eval(remaining)
        return remaining


def main():
    foo = Expression("abba")
    print(foo)
    print(foo.eval("faust"))
    print(foo.eval("abba"))
    print(foo.eval("abbab"))

    print()

    bar = Expression("ab*a")
    print(bar)
    print(bar.eval("abbbbbbbbbbba"))
    print(bar.eval("aa"))
    print(bar.eval("anaconda"))


if __name__ == '__main__':
    main()