#!/usr/bin/env python3
"""
"""

import sys
import os
import argparse
import shutil

def main():
	"""
	"""

	return 42

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
