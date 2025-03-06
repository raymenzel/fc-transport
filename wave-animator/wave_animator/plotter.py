from argparse import ArgumentParser
from functools import partial

import matplotlib.animation as animation
import matplotlib.pyplot as plt


def animate(frame, axis):
    """Plot the next frame on input axis.

    Args:
        frame: Tuple of (x, y, time) data.
        axis: Axis to plot on.

    Returns:
        Lines2D object that was plotted.
    """
    x, y, time = frame
    plt.cla()
    axis.set_title(f"Time: {time:04.2f} s", fontfamily="monospace")
    axis.set_xlim(0, 1000)
    axis.set_xlabel("x [meters]")
    axis.set_ylim(0, 30)
    axis.set_ylabel("Density [kg m-3]")
    line, = axis.plot(x, y)
    return line,


def parse(path):
    """Parses the snapshot csv file and returns the data for the next frame.

    Args:
        path: String path to the csv snapshot file.

    Returns:
        List of x values, density values, and the simulation time for the frame.
    """
    with open(path) as csv:
        current_time = 0.
        times = set([current_time])
        x_values = []
        rho_values = []
        for line in csv:
            time, x, v, rho = [float(i.strip()) for i in line.split(",")]
            if time not in times:
                yield x_values, rho_values, current_time
                current_time = time
                times.add(time)
                x_values = [x,]
                rho_values = [rho,]
            else:
                x_values.append(x)
                rho_values.append(rho)
        yield x_values, rho_values, current_time


def cli():
    """Entry point for the CLI."""
    # Define and parse the command line arguments.
    parser = ArgumentParser(description="Turns fluid snapshots into gifs.")
    parser.add_argument("csv_file", help="Path to a snapshot csv file.")
    parser.add_argument("-o", "--output", help="Path to the output gif.",
                        type=str, default="wave.gif")
    args = parser.parse_args()

    # Create a matplotlib.pyplot Figure and Axis.
    figure, axis = plt.subplots()

    # Create and save the gif.
    gif = animation.FuncAnimation(
        figure,
        partial(animate, axis=axis),
        frames=parse(args.csv_file),
        cache_frame_data=False,
        interval=50,
    )
    gif.save(args.output, writer="pillow")
