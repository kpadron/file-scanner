#!/usr/bin/env python3
"""
"""


import sys
import os
import shutil
import tempfile
import gzip
import hashlib


class FileInfo:
    """
    """

    def __init__(self, name = None, size = None, mtime = None, hash = None, flags = None):
        """
        Initialize self as a fileinfo object.
        """

        self.name = ''
        self.size = 0
        self.mtime = 0
        self.hash = 0
        self.flags = 0

        if name:
            self.name = name

        if size:
            self.size = size

        if mtime:
            self.mtime = mtime

        if hash:
            self.hash = hash

        if flags:
            self.flags = flags

    def __str__(self):
        """
        Returns string representation of self.
        """

        s = str(self.name) + '\n'
        s += str(self.size) + '\n'
        s += str(self.mtime) + '\n'
        s += str(self.hash) + '\n'
        s += str(self.flags) + '\n'

        return s


def file_hash(path):
    """
    Return hash generated from file's contents.
    """

    HASH_BLOCKSIZE = 1 << 20
    hash = hashlib.sha256()

    with open(path, 'rb') as f:
        while True:
            data = f.read(HASH_BLOCKSIZE)

            if not data:
                break

            hash.update(data)

        h = hash.hexdigest()

    return h


def tree_parse(path, table):
    """
    Recursively scan directories and add fileinfo objects to table.
    """

    path = os.path.abspath(path)

    if os.path.isfile(path):
        stat = os.stat(path)
        info = FileInfo(path, stat.st_size, stat.st_mtime)
        table[path] = info
        return

    with os.scandir(path) as dir:
        for entry in dir:
            if entry.is_file():
                stat = entry.stat()
                info = FileInfo(entry.path, stat.st_size, stat.st_mtime)
                table[entry.path] = info
            elif entry.is_dir():
                tree_parse(entry.path, table)


def filelog_read(path):
    """
    Read log and extract fileinfo objects from it.
    """

    table = {}

    try:
        with gzip.open(path, 'rt') as zf:
            for i, line in enumerate(zf.read().splitlines()):
                if i % 5 == 0:
                    info = FileInfo()
                    info.name = str(line)
                elif i % 5 == 1:
                    info.size = int(line)
                elif i % 5 == 2:
                    info.mtime = float(line)
                elif i % 5 == 3:
                    info.hash = int(line, 16)
                else:
                    info.flags = int(line)
                    table[info.name] = info
    except FileNotFoundError:
        pass

    return table


def filelog_write(path, table):
    """
    Write log and pack fileinfo objects into it.
    """

    with gzip.open(path, 'wt') as zf:
        for _, v in table.items():
            zf.write(str(v))


def fileinfo_diff(table):
    """
    Scan files that appear in the table and determine if they are updated.
    """

    updated = []

    # Scan files from table to see if they are different
    for k, v in table.items():
        stat = os.stat(k)

        # File appears to be different
        if (v.size != stat.st_size or v.mtime < stat.st_mtime)
            # Generate new hash of file's contents
            h = file_hash(k)
            v.flags |= 0x1
            updated.append((0, k))
        # File appears to be the same
        else:
            v.flags &= ~0x1

    return updated


def fileinfo_archive(path, updated):
    """
    Archive files that are marked as new in archive located at path.
    """

    # Create temporary directory
    with tempfile.TemporaryDirectory() as dir:
        # Extract current archive to temp directory
        try:
            shutil.unpack_archive(path, dir, 'gztar')
        except:
            print('error opening archive')

        # Copy new or updated files to temp directory
        for _, file in table.items():
            # perform path name correction
            if os.path.isabs(file.name):
                dst = os.path.join(dir, os.path.normpath(os.path.relpath(file.name, os.path.sep)))
            else:
                dst = os.path.join(dir, os.path.normpath(file.name))

            # Copy file overwriting existing data
            if file.flags & 0x1:
                try:
                    shutil.copy(file.name, dst)
                except IOError:
                    os.makedirs(os.path.dirname(dst))
                    shutil.copy(file.name, dst)

                file.flags &= ~0x1

        # Create new archive from temp directory
        shutil.make_archive(os.path.splitext(os.path.splitext(path)[0])[0], 'gztar', dir)



if __name__ == '__main__':
    """
    """

    SCANNER_LOG = os.path.join(os.path.expanduser('~'), '.fslog.gz')
    SCANNER_ARCHIVE = os.path.join(os.path.expanduser('~'), '.fsarchive.tar.gz')

    argc = len(sys.argv)
    if argc < 2:
        print('Usage: %s <filepath> ...' % (sys.argv[0]))
        sys.exit(1)

    # Read file info from log
    print('Reading log...')
    old_table = filelog_read(SCANNER_LOG)
    print('  entries read: %d' % (len(old_table)))

    # Add any new files to list
    print('Adding new files...')
    new_table = {}
    for i in range(1, argc):
        tree_parse(sys.argv[i], new_table)

    print('  files added: %d' % (len(new_table)))

    # Combine old and new table with old entries overwriting new ones
    table = {**new_table, **old_table}

    # Scan files for changes or updates
    print('Scanning files for changes...')
    updated = fileinfo_diff(table)
    print('  files changed: %d' % (len(updated)))

    # Add new or updated files to archive, remove old references
    print('Archiving files...')
    fileinfo_archive(SCANNER_ARCHIVE, updated)

    # Write updated log info to file
    print('Writing new log...')
    filelog_write(SCANNER_LOG, table)

    print('Done')
    sys.exit(0)
