/*
  CSC 225 - Summer 2018
  Assignment 3 - Ladder.java
  Graeme Prendergast, V00823043

  Based on the provided starter code

  This program takes as input a file containing a list of words, and then a start
  and end word in that file to construct a "Word Ladder" between.

  From Wikipedia: A word ladder puzzle begins with two words, and to solve the puzzle
  one must find a chain of other words to link the two, in which two adjacent words
  (that is, words in successive steps) differ by one letter.

  So for example, a valid word ladder for BASH -> BATE could be:
  BASH -> BATH -> BATE, assuming all these words are contained in the input file.

  This program always finds an optimal world ladder (if there is one to be found)
  by constructing a graph and then using breadth-first search to find a valid
  path from start->end.
*/

import java.io.*;
import java.util.*;

public class Ladder
{
  public static void showUsage()
  {
	   System.err.printf("Usage: java Ladder <word list file> <start word> <end word>\n");
	}

	public static void main(String[] args)
  {
		//At least four arguments are needed
		if (args.length < 3)
    {
			showUsage();
			return;
		}
		String wordListFile = args[0];
		String startWord = args[1].trim();
		String endWord = args[2].trim();

    if(startWord.length() != endWord.length())
    {
      System.out.printf("No word ladder found.\n");
      return;
    }

		//Read the contents of the word list file into a LinkedList (requires O(nk) time for
		//a list of n words whose maximum length is k).
		//(Feel free to use a different data structure)
		BufferedReader br = null;
		LinkedList<String> words = new LinkedList<String>();

		try
    {
			br = new BufferedReader(new FileReader(wordListFile));
		}
    catch( IOException e )
    {
			System.err.printf("Error: Unable to open file %s\n",wordListFile);
			return;
		}

		try
    {
			for (String nextLine = br.readLine(); nextLine != null; nextLine = br.readLine())
      {
				nextLine = nextLine.trim();
				if (nextLine.equals(""))
					continue; //Ignore blank lines
				//Verify that the line contains only lowercase letters
				for(int ci = 0; ci < nextLine.length(); ci++)
        {
					//The test for lowercase values below is not really a good idea, but
					//unfortunately the provided Character.isLowerCase() method is not
					//strict enough about what is considered a lowercase letter.
					if ( nextLine.charAt(ci) < 'a' || nextLine.charAt(ci) > 'z' )
          {
						System.err.printf("Error: Word \"%s\" is invalid.\n", nextLine);
						return;
					}
				}
				words.add(nextLine);
			}
		}
    catch (IOException e)
    {
			System.err.printf("Error reading file\n");
			return;
		}



		/* Find a word ladder between the two specified words.
    Ensure that the output format matches the assignment exactly. */

    // Convert out input words into a list of vertices, takes O(n) time
    // This ignores all words that aren't the length of startWord to speed things up by a constant factor
    ArrayList<Vertex> vertexList = new ArrayList<>();
    ListIterator<String> wordsIterator = words.listIterator(0);
    while( wordsIterator.hasNext() )
    {
      String word = wordsIterator.next();
      if( word.length() == startWord.length() )
        vertexList.add(new Vertex(word));
    }

    // Here's where we're constructing our adjacency list in O(nlogn) time.
    // This scheme sorts our input list k times, for k chars in the relevent word length,
    // ignoring word.charAt(k) to check for adjacency, then iteratres through each sorted
    // list in linear time, adding to each vertex's neighbours dynamically as we go along.
    // Since k is bounded by a constant (100), and we're using TimSort (nlogn), we can say
    // this whole blocks is O(nlogn).
    for(int i=0; i<startWord.length(); i++)
    {
      VertexComparator comp = new VertexComparator();
      comp.ignore(i);
      Collections.sort(vertexList, comp); // Java's Collections.sort uses a modified TimSort

      ArrayList<Vertex> tempAdj = new ArrayList<>();
      int lastComponentStart = 0;
      int index = 0;
      while( index < vertexList.size() )
      {
        if( index == 0 )
        {
          lastComponentStart = 0;
          tempAdj = new ArrayList<>();
        }
        else if( compareVertex(vertexList.get(index), vertexList.get(index-1), i) == false )
        {
          // Add tempAdj to each of the previous component's adjLists'
          for(int k=index-1; k>=lastComponentStart; k--)
          {
            vertexList.get(k).addNeighbourList(tempAdj);
          }
          lastComponentStart = index;
          tempAdj = new ArrayList<>();
        }
        // If neither of these ^^ ifs run, vertexList[index] is adj to vertexList[index-1]
        tempAdj.add(vertexList.get(index));
        index++;
      }
      for(int k=index-1; k>=lastComponentStart; k--)
      {
        vertexList.get(k).addNeighbourList(tempAdj);
      }
    }

    // As an aside, with this scheme each vertex is going to list itself as a neighbour k times,
    // which is weird and not normal for an adjList, but this doesn't impact our O(n+m) running time
    // for BFS since again, k is bounded by the max length of words permitted as input (100).

    // Here's our BFS
    ArrayList<Vertex> shortestPath = findShortestPath(vertexList, startWord, endWord);

    // Here's where we print out the relevent output
    if( shortestPath.size() == 0 )
      System.out.printf("No word ladder found.\n");
    for(int i=shortestPath.size()-1; i>=0; i--)
    {
      System.out.println(shortestPath.get(i).word);
    }
    //System.out.printf("%d words\n", shortestPath.size());

	}


  // Perform BFS rooted at the start word until we find the end word, or we
  // run out of nodes in our connected component, in which case there is no path.
  // Then, return the list created by backtracking through that path, or an empty list if no path exists.
  // Since this is based on vanilla BFS, it hits O(n+m) time complexity.
  private static ArrayList<Vertex> findShortestPath(ArrayList<Vertex> adjList, String start, String end)
  {
    // Find startWord's vertex in adjList, this takes O(n) time
    Vertex startVertex = null;
    for(int i=0; startVertex == null; i++)
    {
      if( adjList.get(i).word.compareTo(start) == 0 )
        startVertex = adjList.get(i);
    }

    // Here's where the meat of our BFS-like algorithm is
    // We're using a LinkedList for our queue which lets us enqueue(addLast)
    // and dequeue(removeFirst) in constant time since we have pointers to both.
    ArrayList<Vertex> shortestPath = new ArrayList<>();
    LinkedList<Vertex> bfsQueue = new LinkedList<>();
    bfsQueue.addLast(startVertex);
    startVertex.visit();
    startVertex.setParent(startVertex);
    while( bfsQueue.size() != 0 )
    {
      Vertex v = bfsQueue.removeFirst();
      if( v.word.compareTo(end) == 0 )
      {
        Vertex parentTracer = v;
        while( parentTracer.word.compareTo(start) != 0 )
        {
          shortestPath.add(parentTracer);
          parentTracer = parentTracer.parent;
        }
        shortestPath.add(parentTracer);
        break;
      }

      for(int i=0; i<v.neighbours.size(); i++)
      {
        if( v.neighbours.get(i).visited == false )
        {
          v.neighbours.get(i).visit();
          v.neighbours.get(i).setParent(v);
          bfsQueue.addLast(v.neighbours.get(i));
        }
      }
    }

    return shortestPath;
  }

  // Checks if two vertices are "equal" if we ignore one letter, as in the sorting
  private static boolean compareVertex(Vertex u, Vertex v, int ignoreSlot)
  {
    for(int i=0; i<u.word.length(); i++)
    {
      if( i != ignoreSlot )
      {
        if( u.word.charAt(i) != v.word.charAt(i) )
          return false;
      }
    }
    return true;
  }


  // Prints out our adjList for debugging purposes
  private static void testPrint(ArrayList<Vertex> adjList)
  {
    for(int i=0; i<adjList.size(); i++)
    {
      System.out.printf("%s: { ", adjList.get(i).word);
      for(int j=0; j<adjList.get(i).neighbours.size(); j++)
        System.out.printf("%s, ", adjList.get(i).neighbours.get(j).word);
      System.out.printf("}\n");
    }
  }
}



// Comparator used to sort words while ignoring 1 char slot per sort
class VertexComparator implements Comparator<Vertex>
{
  private static int ignoreSlot;

	public int compare(Vertex u, Vertex v)
	{
		for(int i=0; i<u.word.length(); i++)
		{
      if( i != ignoreSlot )
      {
  			if(u.word.charAt(i) - v.word.charAt(i) != 0)
  				return u.word.charAt(i) - v.word.charAt(i);
      }
		}

		return 0; //If we get through the loop, it means the words are equal
	}

  // How we handle ignoring a given char slot
  public void ignore(int i)
  {
    this.ignoreSlot = i;
  }
}



// Object-oriented way of keeping all the information for each vertex bundled
// up in a neat little package. Setters/getters, nothing interesting to see here.
class Vertex
{
  public String word;
  public boolean visited;
  public ArrayList<Vertex> neighbours;
  public Vertex parent;

  public Vertex(String word)
  {
    this.word = word;
    this.visited = false;
    this.neighbours = new ArrayList<>();
    this.parent = null;
  }

  public void addNeighbour(Vertex v)
  {
    neighbours.add(v);
  }

  public void addNeighbourList(ArrayList<Vertex> newNeighbours)
  {
    for(int i=0; i<newNeighbours.size(); i++)
    {
      this.neighbours.add(newNeighbours.get(i));
    }
  }

  public void visit()
  {
    this.visited = true;
  }

  public void setParent(Vertex v)
  {
    this.parent = v;
  }
}
