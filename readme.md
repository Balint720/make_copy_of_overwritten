## Console program for making backups
# Introduction
This console program written in C can be used to make backups of files found in a directory that would be replaced by a file from another folder.

An example is when installing a mod for a video game: the mod author provides modified files of the game binaries, which replace the original ones.

Useful when installing mods that replace a significant portion of files, but not the whole thing: it would be cumbersome to comb through which files have to be backed up or not.

# Usage
You have to provide 3 file paths (can be relative based on install location):

1. Path to directory that contains the files that will replace the original ones.
2. Path to directory that contains the original files.
3. Path to backup directory.

The original files that would be replaced are copied into the backup directory.
The program will ask if you want the backup directory to be erased. If not, then note that files in the backup folder that match ones in the original folder might be replaced.