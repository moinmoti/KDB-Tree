import plotly.graph_objects as go

height = 0
nodes = []
tolerance = []
with open("Snapshots/KDBTree.csv") as f:
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

fig = go.Figure()

# Set axes properties
fig.update_xaxes(
    range=[-182, 182],
    showgrid=False,
    zeroline=False,
    mirror=True,
    # ticks="outside",
    showticklabels=False,
    showline=False,
    linecolor="black",
)
fig.update_yaxes(
    range=[-91, 91],
    showgrid=False,
    zeroline=False,
    mirror=True,
    # ticks="outside",
    showticklabels=False,
    showline=False,
    linecolor="black",
)

perimeter = 0

# Add shapes
for n in nodes:
    fig.add_trace(
        go.Scatter(
            mode="lines",
            x=[n[0], n[0], n[2], n[2], n[0]],
            y=[n[1], n[3], n[3], n[1], n[1]],
            line=dict(color="black", width=0.75),
        )
    )
    perimeter += 2 * ((n[2] - n[0]) + (n[3] - n[1]))

numNodes = len(nodes)
avgLen = perimeter / numNodes
avgCardinality = 1e7 / numNodes
print(
    f"Total Number of Nodes: {numNodes}\nAverage Perimeter: {avgLen:.2f}\nAverage Cardinality: {avgCardinality:.2f}"
)

""" fig.add_trace(
    go.Scatter(
        x=[None],
        y=[None],
        mode="markers",
        marker=dict(
            cmin=0,
            cmax=1,
            colorscale=[[0, "rgba(255,143,0,0)"], [1, "rgba(255,143,0,1)"]],
            showscale=True,
            colorbar=dict(
                title="Node tolerance",
                tickvals=[0, meanTol, 1],
                ticktext=[
                    str(minTol) + " (Min)",
                    str(meanTol) + " (Mean)",
                    str(maxTol) + " (Max)",
                ],
                ticks="outside",
            ),
        ),
    )
) """

fig.update_layout(
    showlegend=False,
    plot_bgcolor="white",
    margin=dict(l=0, r=0, t=0, b=0),
    height=720,
    width=1440,
)
fig.write_image(file="Snapshots/KDBTree.png")
