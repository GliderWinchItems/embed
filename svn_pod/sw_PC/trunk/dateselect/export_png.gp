# This exports the plot to an output terminal, then restores the X11 (previous terminal)
set terminal push # save the current terminal settings
set terminal pngcairo  # change terminal to PNG
set output "$0"   # set the output filename to the first option
replot            # repeat the most recent plot command
set output        # restore output to interactive mode
set terminal pop  # restore the terminal

