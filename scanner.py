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
        s += hex(self.hash) + '\n'
        s += hex(self.flags) + '\n'

        return s


def file_hash(path):
    """
    Return hash generated from file's contents.
    """

    HASH_BLOCKSIZE = 1 << 20
    hash = hashlib.md5()

    with open(path, 'rb') as f:
        while True:
            data = f.read(HASH_BLOCKSIZE)

            if not data:
                break

            hash.update(data)

        h = int(hash.hexdigest(), 16)

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


def tree_remove(path, remove_root = True):
    """
    Recursively remove emptry directories.
    """

    path = os.path.abspath(path)

    # Skip normal files
    if not os.path.isdir(path):
        return

    # Remove empty subdirectories
    for file in os.listdir(path):
        tree_remove(os.path.join(path, file))

    # Remove path directory if it is empty and root flag
    if not len(os.listdir(path)) and remove_root:
        os.rmdir(path)


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
                    info.hash = int(line, 0)
                else:
                    info.flags = int(line, 0)
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


def fileinfo_add(log_table, add_table):
    """
    Add files to log table from add table.

    Params:
        log_table {fileinfo} - dictionary to add to
        add_table {fileinfo} - dictionary to add from

    Return:
        added (int) - number of entries added
    """

    added = 0

    for k in add_table:
        if k not in log_table:
            log_table[k] = add_table[k]
            log_table[k].flags |= 0x1
            log_table[k].flags &= ~0x2
            log_table[k].hash = file_hash(k)
            added += 1
        elif log_table[k].flags & 0x2:
            log_table[k].flags &= ~0x2
            added += 1

    return added


def fileinfo_remove(log_table, remove_table):
    """
    Remove files from log table that are in remove table.

    Params:
        log_table    {fileinfo} - dictionary to remove from
        remove_table {fileinfo} - dicionary containing keys to remove

    Returns:
        removed (int) - number of entries removed

    Notes:
    does not actually remove entries simply marks for deletion (see fileinfo_clean)
    """

    removed = 0

    for k in remove_table:
        if k in log_table:
            log_table[k].flags |= 0x2 # marked for removal
            removed += 1

    return removed


def fileinfo_scan(table):
    """
    Scan files that appear in the table and determine if they are updated.

    Params:
        table {fileinfo} - dictionary containing fileinfo objects to diff

    Returns:
        updated (int) - number of entries that have changed

    Notes:
    also marks for deletion any non-existent files that appear in the table
    """

    updated = 0

    # Scan files from table to see if they are different
    for k, v in table.items():
        try:
            stat = os.stat(k)

            # File appears to be different
            if v.size != stat.st_size or v.mtime != stat.st_mtime:
                # Generate new hash of file's contents
                v.hash = file_hash(v.name)
                v.flags |= 0x1 # recently updated
                updated += 1

        # Remove non-existent entries
        except FileNotFoundError:
            v.flags |= 0x2

    return updated


def fileinfo_clean(table):
    """
    Delete entries in table that have been previously marked for removal.

    Params:
        table {fileinfo} - table to delete entries from

    Returns:
        deleted (int) - number of entries deleted
    """

    deleted = 0

    for k, v in {**table}.items():
        if v.flags & 0x2:
            table.pop(k, None)
            deleted += 1

    return deleted


def fileinfo_archive(path, table):
    """
    Archive files that are marked as new in archive located at path.
    """

    archived = 0

    # Create temporary directory
    with tempfile.TemporaryDirectory() as dir:
        # Extract current archive to temp directory
        try:
            shutil.unpack_archive(path, dir, 'gztar')
        except FileNotFoundError:
            pass

        # Copy new or updated files to temp directory
        for _, file in table.items():
            # perform path name correction
            dst = os.path.join(dir, os.path.normpath(os.path.relpath(file.name, os.path.sep)))

            # Remove file from archive
            if file.flags & 0x2:
                try:
                    os.remove(dst)
                except FileNotFoundError:
                    pass

            # Copy file overwriting existing data
            elif file.flags & 0x1:
                try:
                    shutil.copy(file.name, dst)
                except IOError:
                    os.makedirs(os.path.dirname(dst))
                    shutil.copy(file.name, dst)

                file.flags &= ~0x1
                archived += 1

        # Remove empty directories
        tree_remove(dir, False)

        # Create new archive from temp directory
        shutil.make_archive(os.path.splitext(os.path.splitext(path)[0])[0], 'gztar', dir)

    return archived


if __name__ == '__main__':
    """
    """

    import argparse

    parser = argparse.ArgumentParser()
    parser.add_argument('-v', '--verbosity', action='count', default=0,
                        help='increase output verbosity')
    parser.add_argument('-a', '--add-files', nargs='+', default=[],
                        help='list of files or directories to add to backup list')
    parser.add_argument('-r', '--remove-files', nargs='+', default=[],
                        help='list of files or directories to remove from backup list')
    parser.add_argument('-u', '--update-files', action='store_true', default=False,
                        help='scan files on backup list and check for changes or updates, \
                        perform backup if necessary')
    args = parser.parse_args()

    print(args)

    # SCANNER_LOG = os.path.join(os.path.expanduser('~'), '.fslog.gz')
    # SCANNER_ARCHIVE = os.path.join(os.path.expanduser('~'), '.fsarchive.tar.gz')
    SCANNER_LOG = '.fslog.gz'
    SCANNER_ARCHIVE = '.fsarchive.tar.gz'
    add_table = {}
    remove_table = {}
    log_table = {}

    # Read file info from log
    if args.verbosity > 0:
        print('Reading log...')

    log_table = filelog_read(SCANNER_LOG)
    if args.verbosity > 1:
        print('  entries read: %d' % (len(log_table)))

    # Add any new files to list
    if args.add_files:
        if args.verbosity > 0:
            print('Adding new files...')

        for p in args.add_files:
            tree_parse(p, add_table)

        added = fileinfo_add(log_table, add_table)

        if args.verbosity > 1:
            print('  files added: %d' % (added))

    # Remove any new files from the list
    if args.remove_files:
        if args.verbosity > 0:
            print('Removing files...')

        for p in args.remove_files:
            tree_parse(p, remove_table)

        removed = fileinfo_remove(log_table, remove_table)

        if args.verbosity > 1:
            print('  files removed: %d' % (removed))

    # Scan files for changes or updates
    if args.update_files:
        if args.verbosity > 0:
            print('Scanning files for changes...')

        updated = fileinfo_scan(log_table)

        if args.verbosity > 1:
            print('  files changed: %d' % (updated))

    # Add new or updated files to archive, remove old references
    if args.update_files:
        if args.verbosity > 0:
            print('Archiving files...')

        archived = fileinfo_archive(SCANNER_ARCHIVE, log_table)
        deleted = fileinfo_clean(log_table)

        if args.verbosity > 1:
            print('  files archived: %d' %(archived))
            print('  files deleted: %d' %(deleted))

    # Write updated log info to file
    if args.verbosity > 0:
        print('Writing new log...')

    filelog_write(SCANNER_LOG, log_table)

    if args.verbosity > 1:
        print('  entries written: %d' % (len(log_table)))

    if args.verbosity > 0:
        print('Done')

    sys.exit(0)
