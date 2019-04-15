# This parser is largely just a translation of Matt Might's recursiev descent Regex parser 
# from Java to Python (http://matt.might.net/articles/parsing-regex-with-recursive-descent/)
#
# While my original plan was to use this parser to implement some basic regex functions 
# (ie find/match), it turns out parsing regexes is way more complicated than I thought it was,
# so I'm leaving this as a half-complete project to expand on later. The parser itself works
# fine though, parsing a given input regex into a tree of literals.
#
# Written by Graeme Prendergast, Spring 2019

class RegexParser:
    def __init__(self, s):
        self.input = s

    def peek(self):
        return self.input[0]

    def eat(self, char):
        if self.peek() == char:
            self.input = self.input[1:]
        else:
            raise Exception("Expected %s , got %s\n" % (char,peek()) )

    def next(self):
        nextChar = self.peek()
        self.eat(nextChar)
        return nextChar

    def hasMore(self):
        return ( len(self.input) > 0 )

    def regex(self):
        currentTerm = self.term()
        if ( self.hasMore() and self.peek() == '|' ):
            self.eat('|')
            currentRegex = self.regex()
            return Choice(currentTerm,currentRegex)
        else:
            return currentTerm

    def term(self):
        currentFactor = 'BLANK'
        while ( self.hasMore() and self.peek() != ')' and self.peek() != '|' ):
            nextFactor = self.factor()
            currentFactor = Sequence(currentFactor, nextFactor)
        return currentFactor

    def factor(self):
        b = self.base()
        while ( self.hasMore() and self.peek() == '*' ):
            self.eat('*')
            b = Repetition(b)
        return b

    def base(self):
        if ( self.peek() == '('):
            self.eat('(')
            r = self.regex()
            self.eat(')')
            return r
        else:
            return Primitive(self.next())

class Primitive:
    def __init__(self, c):
        self.char = c

class Choice:
    def __init__(self, a, b):
        self.c1 = a
        self.c2 = b

class Sequence:
    def __init__(self, current, next):
        self.first = current
        self.second = next

class Repetition:
    def __init__(self, i):
        self.internal = i

class Primitive:
    def __init__(self, c):
        self.char = c

def main():
    testinput = "ab*a"
    parser = RegexParser(testinput)

if __name__ == "__main__":
    main()
