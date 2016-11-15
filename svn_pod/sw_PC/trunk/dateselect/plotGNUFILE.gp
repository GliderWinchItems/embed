# This expects the environmental variable GNUFILE to be set with the file name (no quotes) to be plotted

# Set two y axis plot
set ytics nomirror
set y2tics
set key top left reverse Left

# X-axis tic every 64 data points
set xtics rotate

# Turn on x and y grid lines
set grid

# Left y axis is tension, Right y axis is acceleration vector magnitude
set ylabel "tension (grams)"; set y2label "accelerometer |vector| (g's)"

# X axis label
set xlabel "time (seconds)"
set x2label "FILE NAME: `echo $GNUFILE`  DURATION (secs): `echo $GNUSEC`"

# Do the plot
plot "`echo $GNUFILE`" u 1:2 w lines axes x1y1 t "tension", "" u 1:6 w lines axes x1y2 t"accelerometer"

# Export the file as a .pdf image
call "export_pdf.gp" "`echo $GNUFILE`.pdf"

# Let the op look at until <enter> hit
pause -1
