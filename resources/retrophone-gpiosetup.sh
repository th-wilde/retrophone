#! /bin/bash
echo "CONFIGURE GPIO EXPORTS..."
gpio -g mode 17 up
echo "...BCM-GPIO 17 pullup enabled."
gpio edge 17 both
echo "...BCM-GPIO 17 edge detection (both) enabled."
gpio export 27 out
echo "...BCM-GPIO 27 exported set as output."
echo "...done!"