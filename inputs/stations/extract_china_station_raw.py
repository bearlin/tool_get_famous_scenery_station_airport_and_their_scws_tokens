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
pattern = re.compile("<li><a\ href=\"/wiki/[^U].*title=\".*</a></li>")
strings = re.findall(pattern, rawfile.read())
print "%r" % (strings)

# Processing matched strings
lineCount=0
for string in strings:
  lineCount = lineCount + 1
  print "Processing[%d]:%s" % (lineCount, string)

  strtmp1 = string.rstrip('</a></li>').lstrip()
  #print "after strip end:%s" % (strtmp1)
  keyIdx = strtmp1.index('title=\"')
  startIdx = keyIdx + 7
  endIdx = len(strtmp1)
  #print "keyIdx:%r" % (keyIdx)
  #print "startIdx:%r" % (startIdx)
  #print "endIdx:%r" % (endIdx)
  strtmp2 = strtmp1[startIdx:endIdx]

  keyIdx = strtmp2.index('>')
  startIdx = keyIdx + 1
  endIdx = len(strtmp2)
  #print "keyIdx:%r" % (keyIdx)
  #print "startIdx:%r" % (startIdx)
  #print "endIdx:%r" % (endIdx)
  strtmp3 = strtmp2[startIdx:endIdx]

  strtmp4 = strtmp3.lstrip().rstrip() # strip white space characters
  if strtmp4 == '':
    continue
  print "After stripped:%s" % (strtmp4)
  # Write stripped result to dumpfile
  dumpfile.write(strtmp4)
  dumpfile.write("\n")

dumpfile.close()
rawfile.close()

print "Done."
