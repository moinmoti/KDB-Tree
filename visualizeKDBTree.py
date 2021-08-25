import matplotlib.patches as patches
import matplotlib.pyplot as plt
import sys
from pathlib import Path

height = 0
nodes = []
filename = sys.argv[1]
with open(filename) as f:
    for i, line in enumerate(f):
        if int(line.split(",")[0]) == height:
            nodes.append(
                (
                    float(line.split(",")[2]),
                    float(line.split(",")[3]),
                    float(line.split(",")[4]),
                    float(line.split(",")[5]),
                )
            )
        # if i == 10000: break


# Create figure and axes
fig, ax = plt.subplots(1)
fig.set_figheight(8)
fig.set_figwidth(16)
axes = plt.gca()
axes.set_xlim([-180, 180])
axes.set_ylim([-90, 90])

# Create a Rectangle patch

for l in nodes:
    rect = patches.Rectangle(
        (l[0], l[1]),
        l[2] - l[0],
        l[3] - l[1],
        linewidth=1,
        edgecolor="black",
        facecolor="none",
    )
    ax.add_patch(rect)

# points = {}
# with open('/home/dinosar/rd/learned-indexes/experiments_classification/experiment_0/trainingData.csv') as f:
#   for i, line in enumerate(f):
#      if int(line.split(',')[0]) == 0:
#            if int(line.split(',')[4]) not in points:
#                points[int(line.split(',')[4])] = []
#           x = float(line.split(',')[2])*360-180
#          y = float(line.split(',')[3])*180-90
#           points[int(line.split(',')[4])].append((x,y))

# for model in points:
#   ax.scatter([x[0] for x in points[model]], [y[1] for y in points[model]], color=(r.uniform(0,1),r.uniform(0,1),r.uniform(0,1)), alpha = 1, s = 0.5)

plt.savefig("Snapshots/" + Path(filename).stem + "-" + str(height) + ".png")
