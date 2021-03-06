/*
	Aggregate.java - A command line SQL-like interpreter.

	Given a CSV file as input, a column name, and a function to perform on that column
	(count/sum/avg/count_distinct), this program performs that aggregation and then
	outputs the resulting table generated by this aggregation.

	Written by Graeme Prendergast, Summer 2018
*/

import java.util.*;
import java.io.File;

public class Aggregate
{
	public static void main(String[] args)
	{
		if(args.length < 4)
		{
			System.out.printf("Please enter a valid set of arguments.\n");
			System.out.printf("Proper format: <function> <aggregation column> <input file> <group columns ... >\n");
			return;
		}

		String function = args[0];
		String aggregationColumn = args[1];
		String fileName = args[2];
		ArrayList<String> groupColumns = new ArrayList<String>();
		for(int i=3; i<args.length; i++)
			groupColumns.add(args[i]);

		ArrayList<String[]> inputTable = readTable(fileName);

		ArrayList<String[]> dataTable = trimTable(inputTable, aggregationColumn, groupColumns);

		ArrayList<String[]> outputTable;
		switch(function)
		{
			case "count":	outputTable = aggregateCount(dataTable);
										break;
			case "sum":		outputTable = aggregateSum(dataTable);
										break;
			case "avg":		outputTable = aggregateAvg(dataTable);
										break;
			case "count_distinct":
										outputTable = aggregateCountDistinct(dataTable);
										break;
			default:		System.out.printf("Invalid function name: %s\n", function);
									return;
		}

		print(outputTable);

		return;
	}

	public static ArrayList<String[]> readTable(String fileName)
	{
		Scanner reader;
		ArrayList<String[]> inputTable = new ArrayList<String[]>();

		try
		{
			reader = new Scanner(new File(fileName));
		}
		catch(java.io.FileNotFoundException e)
		{
			System.out.printf("Unable to open file: %s\n", fileName);
			return null;
		}

		// This loop is O(n) for n number of rows in the input
		while(reader.hasNextLine())
		{
			String[] line = reader.nextLine().split(",");
			if(line.length > 1)
				inputTable.add(line);
		}

		return inputTable;
	}

	// Returns a "cleaner" data table with unneeded columns trimmed off
	public static ArrayList<String[]> trimTable(ArrayList<String[]> inputTable,
		String aggregationColumn, ArrayList<String> groupColumns)
	{
		int aggregationColumnIndex = -1;
		ArrayList<Integer> groupColumnIndices = new ArrayList<Integer>();

		// This loop is O(n) for n number of rows in the input
		for(int i=0; i<inputTable.get(0).length; i++)
		{
			if(inputTable.get(0)[i].equals(aggregationColumn))
			{
				aggregationColumnIndex = i;
			}
			else if(groupColumns.contains(inputTable.get(0)[i]))
			{
				groupColumnIndices.add(i);
			}
		}

		// This loop is O(n), since the inner loop is bounded by a constant limit
		// of 100,000 characters at most per row
		ArrayList<String[]> trimmedTable = new ArrayList<String[]>();
		for(int i=0; i<inputTable.size(); i++)
		{
			String[] line = new String[groupColumns.size()+1];
			line[0] = inputTable.get(i)[aggregationColumnIndex];
			for(int k=0; k<groupColumnIndices.size(); k++)
				line[k+1] = inputTable.get(i)[groupColumnIndices.get(k)];

			trimmedTable.add(line);
		}

		return trimmedTable;
	}

	public static ArrayList<String[]> aggregateCount(ArrayList<String[]> dataTable)
	{
		ArrayList<String[]> outputTable = new ArrayList<String[]>();
		ArrayList<String[]> testTable = new ArrayList<String[]>();

		String[] firstLine = new String[dataTable.get(0).length];
		for(int i=0; i<dataTable.get(0).length-1; i++)
			firstLine[i] = dataTable.get(0)[i+1];
		firstLine[dataTable.get(0).length-1] = String.format("count(%s)", dataTable.get(0)[0]);
		outputTable.add(firstLine);

		// This remove() call is O(n), which is annoying, but we only use it once per run
		dataTable.remove(0);
		// Collections.sort() uses a modified TimSort, and has a worst case runtime of nlogn
		// This operation is the rate-determining step of the program's asymptotic runtime
		Collections.sort(dataTable, new LineComparator());

		// This whole loop is O(n), since the inner loop is bounded by a constant limit
		// of 100,000 characters per row. Since our input table is sorted row-wise with
		// respect to the grouping columns, we only need to check the if dataTable[i]
		// has the same grouping columns to the output table's most recent row.
		for(int i=0; i<dataTable.size(); i++)
		{
			boolean duplicate = false;
			int duplicateIndex = -1;

			if(outputTable.size() > 0)
			{
				duplicate = true;
				for(int k=1; k<outputTable.get(0).length; k++)
				{
					if(!(dataTable.get(i)[k].equals(outputTable.get(outputTable.size()-1)[k-1])))
					{
						duplicate = false;
						break;
					}
				}
			}
			if(duplicate == true)
			{
				int count = Integer.parseInt(outputTable.get(outputTable.size()-1)[dataTable.get(0).length-1]);
				outputTable.get(outputTable.size()-1)[dataTable.get(0).length-1] = Integer.toString(count+1);
			}
			else //(duplicate == false)
			{
				String[] line = new String[dataTable.get(i).length];
				int j;
				for(j=0; j<dataTable.get(i).length-1; j++)
					line[j] = dataTable.get(i)[j+1];
				line[j] = "1";
				outputTable.add(line);
			}
		}

		return outputTable;
	}

	public static ArrayList<String[]> aggregateSum(ArrayList<String[]> dataTable)
	{
		ArrayList<String[]> outputTable = new ArrayList<String[]>();

		String[] firstLine = new String[dataTable.get(0).length];
		for(int i=0; i<dataTable.get(0).length-1; i++)
			firstLine[i] = dataTable.get(0)[i+1];
		firstLine[dataTable.get(0).length-1] = String.format("sum(%s)", dataTable.get(0)[0]);
		outputTable.add(firstLine);

		dataTable.remove(0); // Remove the header line from dataTable, O(n)
		Collections.sort(dataTable, new LineComparator()); // TimSort, nlogn

		for(int i=0; i<dataTable.size(); i++)
		{
			boolean duplicate = false;
			int duplicateIndex = -1;

			if(outputTable.size() > 0)
			{
				duplicate = true;
				for(int k=1; k<outputTable.get(0).length; k++)
				{
					if(!(dataTable.get(i)[k].equals(outputTable.get(outputTable.size()-1)[k-1])))
					{
						duplicate = false;
						break;
					}
				}
			}
			if(duplicate == true)
			{
				int sum = Integer.parseInt(outputTable.get(outputTable.size()-1)[dataTable.get(0).length-1]);
				outputTable.get(outputTable.size()-1)[dataTable.get(0).length-1] =
					Integer.toString(sum+Integer.parseInt(dataTable.get(i)[0]));
			}
			else //(duplicate == false)
			{
				String[] line = new String[dataTable.get(i).length];
				int j;
				for(j=0; j<dataTable.get(i).length-1; j++)
					line[j] = dataTable.get(i)[j+1];
				line[j] = dataTable.get(i)[0];
				outputTable.add(line);
			}
		}

		return outputTable;
	}

	public static ArrayList<String[]> aggregateAvg(ArrayList<String[]> dataTable)
	{
		ArrayList<String[]> outputTable = new ArrayList<String[]>();
		ArrayList<Integer> countList = new ArrayList<Integer>();

		String[] firstLine = new String[dataTable.get(0).length];
		for(int i=0; i<dataTable.get(0).length-1; i++)
			firstLine[i] = dataTable.get(0)[i+1];
		firstLine[dataTable.get(0).length-1] = String.format("avg(%s)", dataTable.get(0)[0]);
		outputTable.add(firstLine);

		dataTable.remove(0); // Remove the header line from dataTable, O(n)
		Collections.sort(dataTable, new LineComparator()); // TimSort, nlogn

		for(int i=0; i<dataTable.size(); i++) // O(n)
		{
			boolean duplicate = false;
			int duplicateIndex = -1;

			if(outputTable.size() > 0)
			{
				duplicate = true;
				for(int k=1; k<outputTable.get(0).length; k++)
				{
					if(!(dataTable.get(i)[k].equals(outputTable.get(outputTable.size()-1)[k-1])))
					{
						duplicate = false;
						break;
					}
				}
			}
			if(duplicate == true)
			{
				int sum = Integer.parseInt(outputTable.get(outputTable.size()-1)[dataTable.get(0).length-1]);
				outputTable.get(outputTable.size()-1)[dataTable.get(0).length-1] =
					Integer.toString(sum+Integer.parseInt(dataTable.get(i)[0]));
				countList.set(outputTable.size()-2, countList.get(outputTable.size()-2)+1);
			}
			else //(duplicate == false)
			{
				String[] line = new String[dataTable.get(i).length];
				int j;
				for(j=0; j<dataTable.get(i).length-1; j++)
					line[j] = dataTable.get(i)[j+1];
				line[j] = dataTable.get(i)[0];
				outputTable.add(line);
				countList.add(1);
			}
		}
		// Standalone loop over the rows, O(n)
		for(int i=1; i<outputTable.size(); i++)
		{
			float sum = Float.parseFloat(outputTable.get(i)[outputTable.get(i).length-1]);
			float avg = sum / countList.get(i-1);
			outputTable.get(i)[outputTable.get(i).length-1] = Float.toString(avg);
		}

		return outputTable;
	}

	public static ArrayList<String[]> aggregateCountDistinct(ArrayList<String[]> dataTable)
	{
		ArrayList<String[]> outputTable = new ArrayList<String[]>();
		ArrayList<ArrayList<String>> distinctElementList = new ArrayList<ArrayList<String>>();

		String[] firstLine = new String[dataTable.get(0).length];
		for(int i=0; i<dataTable.get(0).length-1; i++)
			firstLine[i] = dataTable.get(0)[i+1];
		firstLine[dataTable.get(0).length-1] = String.format("count_distinct(%s)", dataTable.get(0)[0]);
		outputTable.add(firstLine);

		dataTable.remove(0); //Remove the header line from dataTable, O(n)
		Collections.sort(dataTable, new LineComparator()); // TimSort, nlogn

		for(int i=0; i<dataTable.size(); i++)
		{
			boolean duplicate = false;
			int duplicateIndex = -1;

			if(outputTable.size() > 0)
			{
				duplicate = true;
				for(int k=1; k<outputTable.get(0).length; k++)
				{
					if(!(dataTable.get(i)[k].equals(outputTable.get(outputTable.size()-1)[k-1])))
					{
						duplicate = false;
						break;
					}
				}
			}
			if(duplicate == true)
			{
				if(!(distinctElementList.get(outputTable.size()-2).contains(dataTable.get(i)[0])))
				{
					int count = Integer.parseInt(outputTable.get(outputTable.size()-1)[dataTable.get(0).length-1]);
					outputTable.get(outputTable.size()-1)[dataTable.get(0).length-1] = Integer.toString(count+1);
					distinctElementList.get(outputTable.size()-2).add(dataTable.get(i)[0]);
				}
			}
			else //(duplicate == false)
			{
				String[] line = new String[dataTable.get(i).length];
				int j;
				for(j=0; j<dataTable.get(i).length-1; j++)
					line[j] = dataTable.get(i)[j+1];
				line[j] = "1";
				outputTable.add(line);
				ArrayList<String> distinctElementLine = new ArrayList<String>();
				distinctElementLine.add(dataTable.get(i)[0]);
				distinctElementList.add(distinctElementLine);
			}
		}

		return outputTable;
	}

	public static void print(ArrayList<String[]> table)
	{
		for(int i=0; i<table.size(); i++)
		{
			for(int k=0; k<table.get(i).length; k++)
			{
				if(k>0)
					System.out.printf(",");
				System.out.printf("%s", table.get(i)[k]);
			}
			System.out.printf("\n");
		}

		return;
	}

}

// We use a comparator so that we can sort our 2d ArrayLists
class LineComparator implements Comparator<String[]>
{
	public int compare(String[] line1, String[] line2)
	{
		int comparison;
		for(int i=1; i<line1.length; i++)
		{
			if(line1[i].compareToIgnoreCase(line2[i]) != 0)
				return line1[i].compareToIgnoreCase(line2[i]);
		}

		return 0; //If we get through the loop, it means the lines are equal
	}
}
