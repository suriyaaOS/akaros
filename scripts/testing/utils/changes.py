#!/usr/bin/python
"""Parses a 'git diff --stat' file to extract what files have been changed as
shown in the diff. Extracts the directory paths of those files, and decides
which parts of AKAROS should be compiled and tested, accordingly.
"""
import sys
import re
import json

REGEX_EXTRACT_PATH_FROM_GIT_DIFF_LINE = r'^(?:\s)*([^\s]*)(?:\s)*\|(?:.*)\n?$'
# Path to file with git diff
DIFF_FILE = sys.argv[1]
# Path to file with JSON definition of akaros components
CONFIG_COMP_COMPONENTS_FILE = sys.argv[2]


"""The following is the definition of the given parts of AKAROS. Each 'part' 
consists of a name (which will be a uique identifier printed out to console) as
key, plus a list of PATHS as content. If any of these paths is modified, then 
we will consider that the full part is affected, and therefore may need to be 
compiled and tested again.

Paths should be written from the root repo folder, not beginning with './' nor
'/', and not ending in '/'.

They will be loaded from CONFIG_COMP_COMPONENTS_FILE (which should be 
config/compilation_components.json or something)
"""
# TODO(alfongj): Fill with real values
akaros_parts = {}

affected_parts = {}

def load_component_config() :
	"""Loads ../config/compilation_components.json object, which contains a
	list of all the different AKAROS compilation parts, along with the paths
	to look for for compiling them
	"""
	config_file = open(CONFIG_COMP_COMPONENTS_FILE, 'r')
	global akaros_parts
	akaros_parts = json.load(config_file)['compilation_components']

def extract_dir(diff_line) :
	"""Given a line from a "git diff --stat" output, it tries to extract a 
	directory from it. 

	If a blank or non-change line is passed, it ignores it and returns nothing.

	If a 'change' line (e.g. ' path/to/file.ext  |   33++ ') is passed, it strips
	the path (not including the file name) and prepends a './' to it and returns it.
	"""
	match = re.match(REGEX_EXTRACT_PATH_FROM_GIT_DIFF_LINE, diff_line) 
	if (match) :
		full_path = './' + match.group(1)
		folder_list = full_path.split('/')[0:-1]
		folder_path = '/'.join(folder_list)
		return folder_path

def check_dir(dir) :
	"""Checks if a given directory should set the state of one of the parts to
	affected.
	"""
	global affected_parts
	for part in akaros_parts :
		affected = part in affected_parts
		paths = akaros_parts[part]['PATHS']
		if (not affected) :
			for path in paths :
				# Checks if a given string contains the given path.
				# e.g., If the path is 'path/to':
					# The regex will match for: 'path/to/file.txt' or 
						# 'path/to/and/subpath/to/file.txt'
					# But not for: 'path/file.txt' nor for 'path/tofile.txt' or
						# 'path/tobananas/file.txt'
				regex = re.compile('^\./%s(?:/.*)*$' % path)
				if (re.match(regex, dir)) :
					affected_parts[part] = True
					break

def main() :
	load_component_config()
	diff_file = open(DIFF_FILE)
	for line in diff_file :
		cur_dir = extract_dir(line)
		if (cur_dir) :
			check_dir(cur_dir)

	print ' '.join(affected_parts)

main()