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
pattern = re.compile(">[0-9]*\.[^0-9a-zA-Z]*</span></td>")
strings = re.findall(pattern, rawfile.read())
print "%r" % (strings)

# Processing matched strings
lineCount=0
for string in strings:
  lineCount = lineCount + 1
  print "Processing[%d]:%s" % (lineCount, string)

  strtmp1 = string.rstrip('</span></td>').lstrip()
  print "after strip end:%s" % (strtmp1)
  dotIdx = strtmp1.index('.')
  startIdx = dotIdx + 1
  endIdx = len(strtmp1)
  print "dotIdx:%r" % (dotIdx)
  print "startIdx:%r" % (startIdx)
  print "endIdx:%r" % (endIdx)
  strtmp2 = strtmp1[startIdx:endIdx]
  strtmp3 = strtmp2.lstrip().rstrip() # strip white space characters
  if strtmp3 == '':
    continue
  print "After stripped:%s" % (strtmp3)
  # Write stripped result to dumpfile
  dumpfile.write(strtmp3)
  dumpfile.write("\n")

dumpfile.close()
rawfile.close()

print "Done."
