# Given a source and destination page on Wikipedia, the goal of the Wiki Game is
# to get from source->destination using only links on each wiki page. This script
# automates this process, taking (source,destination) as input, and using BFS
# to find an optimal s:d path.
#
# Due to the exponential nature of trying to crawl Wikipedia through links,
# this program might take a really long time to run.
#
# Written by Graeme Prendergast, Spring 2019

import re
import sys
import urllib.request

class WikiNode:
    def __init__(self, url, parent):
        self.url = url
        self.parent = parent

def wikiBFS(source,dest):
    print("Beginning to crawl Wikipedia... (this might take a while!)")

    nodeQueue = []
    visited = []

    currentNode = WikiNode(source,source)
    visited.append(currentNode.url)
    while (currentNode.url != dest):
        #print("Visiting: %s" % currentNode.url)
        pageHTML = urllib.request.urlopen(currentNode.url).read().decode('utf-8')
        matches = re.findall('"/wiki/[^:]*?"', pageHTML)

        for match in matches:
            link = "https://en.wikipedia.org%s" % match[1:-1]
            if link not in visited:
                visited.append(link)
                nodeQueue.append(WikiNode(link,currentNode))
        currentNode = nodeQueue.pop(0)

    path = []
    while (currentNode.url != currentNode.parent):
        path.append(currentNode.url)
        currentNode = currentNode.parent
    path.append(currentNode.url)
    return path[::-1]

def main():
    if ( len(sys.argv) < 3 ):
        print("Command line arguments should be given as: <source> <destination>")
        return -1

    sourcePN = "https://en.wikipedia.org/wiki/%s" % (sys.argv[1])
    destPN = "https://en.wikipedia.org/wiki/%s" % (sys.argv[2])

    try:
        path = wikiBFS(sourcePN,destPN)
    except Exception as e:
        print("Exception found while crawling Wikipedia!\n%s" % (e) )
        return -1

    print("Shortest path from \"%s\" to \"%s\" is:" % (sys.argv[1],sys.argv[2]) )
    for page in path:
        print(page)

if __name__ == '__main__':
    main()
