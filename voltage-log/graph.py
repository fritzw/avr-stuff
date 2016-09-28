#!/usr/bin/python

from time import strftime
import sys

port = "/dev/ttyUSB0"

def getTerminalSize():
	import os
	env = os.environ
	def ioctl_GWINSZ(fd):
		try:
			import fcntl, termios, struct, os
			cr = struct.unpack('hh', fcntl.ioctl(fd, termios.TIOCGWINSZ, '1234'))
		except:
			return
		return cr
	cr = ioctl_GWINSZ(0) or ioctl_GWINSZ(1) or ioctl_GWINSZ(2)
	if not cr:
		try:
			fd = os.open(os.ctermid(), os.O_RDONLY)
			cr = ioctl_GWINSZ(fd)
			os.close(fd)
		except:
			pass
	if not cr:
		cr = (env.get('LINES', 25), env.get('COLUMNS', 80))

		### Use get(key[, default]) instead of a try/catch
		#try:
		#	cr = (env['LINES'], env['COLUMNS'])
		#except:
		#	cr = (25, 80)
	return int(cr[1]), int(cr[0])

value = 0
digits = ""
with open(port) as f:
	while True:
		c = f.read(1)
		if not c:
			print("End of file")
			break
		if c == ",":
			try:
				value = int(digits)
				(width, height) = getTerminalSize()
				line = strftime("%Y-%m-%d %H:%M:%S")
				line += " {:>3} ".format(value)
				bar = "*" * int(value / 1023 * (width - len(line)))
				print(line + bar)
				sys.stdout.flush()
			except:
				print("Error decoding: " + digits)
			digits = ""
		else:
			digits = digits + c
