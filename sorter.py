#!/usr/bin/env python3
"""
"""

import sys
import os
import argparse
import shutil
import multiprocessing as mp


def main():
	"""
	"""

	argc = len(sys.argv)
	if argc < 2:
		return 1

	filepath = sys.argv[1]
	file_lines = sum(1 for line in open(filepath, 'r'))

	procs = []
	total_procs = os.cpu_count()

	file = open(filepath, 'r+')

	for i in range(total_procs):
		offset = BLOCK_LOW(i, total_procs, file_lines)
		size = BLOCK_SIZE(i, total_procs, file_lines)
		procs.append(mp.Process(target=sort_section, args=(file, offset, size)))
		procs[i].start()

	for proc in procs:
		proc.join()

	return 0


def sort_section(file, offset, size):
	lines = []

	file.seek(offset)
	for _ in range(size):
		lines.append(file.readline())

	lines.sort()

	file.seek(offset)
	file.writelines(lines)

def print_block(id, p, n):
	print("id: %d, p: %d, n: %d, bl: %d, bh: %d, bs: %d" % (id, p, n, BLOCK_LOW(id, p, n), BLOCK_HIGH(id, p, n), BLOCK_SIZE(id, p, n)))

def BLOCK_LOW(id, p, n):
	"""
	Returns lower global index of process based on rank, total procs, and task size.
	"""

	return id * n // p

def BLOCK_HIGH(id, p, n):
	"""
	Returns upper global index of process based on rank, total procs, and task size.
	"""

	return BLOCK_LOW(id + 1, p, n) - 1

def BLOCK_SIZE(id, p, n):
	"""
	Returns size of local data based on rank, total procs, and task size.
	"""

	return BLOCK_HIGH(id, p, n) - BLOCK_LOW(id, p, n) + 1

if __name__ == '__main__':
	main()
