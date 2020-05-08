import csv

# Remove adjacent duplicates
results = []
reader = csv.reader(open('Seasons_Stats.csv'))

header = next(reader)

count = 0
for row in reader:
    if not results:
        new_row = [count]
        new_row.extend(row[1:])
        results.append(new_row)
        count += 1
    else:
        prev_row = results[count-1]
        if row[1:3] != prev_row[1:3]:
            new_row = [count]
            new_row.extend(row[1:])
            results.append(new_row)
            count += 1

# Write results to a csv

with open('preproc_season_stats.csv', 'w', newline='') as csvfile:
    writer = csv.writer(csvfile)

    writer.writerow(header) # header row
    for entry in results:
        writer.writerow(entry)



