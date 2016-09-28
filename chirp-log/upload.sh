#!/bin/bash
wget --method=PUT --body-file=/home/fritz/avr/chirp-log/timestaped.txt https://webering.eu/chirp/update.php -O /dev/null
