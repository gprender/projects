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
 - Union/Or (The '|' symbol)
'''

class Expression:
    '''A parent expression, comprised of 1 or more sub-expressions.'''
    def __init__(self, pattern):
        self.subexpressions = []
        # Annoyingly, we need a C-style FOR loop here so that we can jump
        # through the pattern string. This becomes necessary when we include
        # parentheses, since we just want to recurse on paren contents.
        i = 0
        while i < len(pattern):
            char = pattern[i]   
            if char == '(': # Make a new expression for the paren's contents
                paren_contents = get_paren_contents(pattern[i:])
                self.subexpressions.append(Expression(paren_contents))
                i += (len(paren_contents) + 2)
            elif char == '*': # Wrap the previous expression in a Star
                prev = self.subexpressions[-1]
                self.subexpressions[-1] = Star(prev)
                i += 1
            else:
                self.subexpressions.append(Primitive(char))
                i += 1

    def __str__(self):
        str_subexprs = [str(expr) for expr in self.subexpressions]
        return f"[ {', '.join(str_subexprs)} ]" 

    def eval(self, string):
        '''Recursively evaluate an expression for the given string.'''
        remaining = string
        for expr in self.subexpressions:
            if remaining == '': # Our input string has run out
                return False
            remaining = expr.eval(remaining)
            if remaining == False: # A subexpression failed to match
                return False
        return remaining


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


def get_paren_contents(string):
    '''Return all the characters inside of set of parentheses.'''
    length = 0
    balance = 0
    for char in string:
        if char == '(':
            balance += 1
        if char == ')':
            balance -= 1
        length += 1
        if balance == 0:
            return string[1:length-1]
    raise Exception("The given pattern has unbalanced parantheses!")


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

    print()

    baz = Expression("(a(ba)*c)*a")
    print(baz)
    print(baz.eval("ababababacabaca"))
    print(baz.eval("aca"))
    print(baz.eval("a"))
    print(baz.eval("ababca"))


if __name__ == '__main__':
    main()