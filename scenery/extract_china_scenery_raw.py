#!/usr/bin/python

from sys import argv
import re
import codecs

print "Start."
script, filename = argv
print "script: %r." % script
print "filename: %r." % filename

print "Opening the rawfile..."
rawfile = codecs.open(filename, 'r', encoding='utf-8')
dumpfile = codecs.open(filename + ".dump", 'w', encoding='utf-8')

# Feed the rawfile into findall(); it returns a list of all the found strings
pattern = re.compile(">\S*</strong></span></h2>")
strings = re.findall(pattern, rawfile.read())
print "%r" % (strings)

# Processing matched strings
for string in strings:
  print "Processing:%s" % (string)
  strtmp1 = string.lstrip('>')
  strtmp2 = strtmp1.rstrip('</strong></span></h2>')
  if strtmp2 == '':
    continue
  print "After stripped:%s" % (strtmp2)
  # Write stripped result to dumpfile
  dumpfile.write(strtmp2)
  dumpfile.write("\n")

dumpfile.close()
rawfile.close()

print "Done."
