#!/bin/bash -e

test_directory=$(realpath "$(dirname "${BASH_SOURCE[0]}")")

# Run the executable.
$test_directory/../square-wave
if [ "$?" -ne "0" ]; then
  echo "square-wave failed."
  exit 1
fi

# Make sure it created the plot.
if [ ! -f "wave-output.csv" ]; then
  echo "square-wave failed to produce the output csv file."
  exit 1
fi

# Compare the log file to the expected one.
$test_directory/../compare_logs.x wave-output.csv $test_directory/wave-output.csv
if [ "$?" -ne "0" ]; then
  echo "square-wave did not produce the expected data."
  exit 1
fi

# Generate a .gif
wave-animator $test_directory/../wave-output.csv
if [ "$?" -ne "0" ]; then
  echo "wave-animator failed."
  exit 1
fi

# Make sure it created the gif.
if [ ! -f "wave.gif" ]; then
  echo "wave-animator failed to produce the output csv file."
  exit 1
fi

echo "All tests passed."
