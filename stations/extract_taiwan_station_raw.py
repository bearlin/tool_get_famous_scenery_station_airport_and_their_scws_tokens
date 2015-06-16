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
pattern = re.compile("<li><a\ href=\"/wiki/[^U].*title=\".*</a>")
strings = re.findall(pattern, rawfile.read())
print "%r" % (strings)

# Processing matched strings
lineCount=0
for string in strings:
  lineCount = lineCount + 1
  print "Processing[%d]:%s" % (lineCount, string)

  strtmp1 = string
  # print "len(strtmp1):%r" % (len(strtmp1))
  keyIdxNext = strtmp1.find('title=\"', 0)
  while (keyIdxNext != -1):
    startIdx = keyIdxNext + 7
    keyIdxEnd = strtmp1.find('\">', startIdx)
    endIdx = keyIdxEnd
    # print "keyIdxNext01:%r" % (keyIdxNext)
    # print "keyIdxEnd:%r" % (keyIdxEnd)
    # print "startIdx:%r" % (startIdx)
    # print "endIdx:%r" % (endIdx)
    strtmp2 = strtmp1[startIdx:endIdx]
    print "Found token:%s" % (strtmp2)

    # Try to find next keyIdxNext
    keyIdxNext = strtmp1.find('title=\"', endIdx)
    # print "keyIdxNext02:%r" % (keyIdxNext)

    if (strtmp2[0] == 'C') :
      continue

    # Write stripped result to dumpfile
    dumpfile.write(strtmp2)
    dumpfile.write("\n")

  # if (lineCount == 3) :
  #   break

dumpfile.close()
rawfile.close()

print "Done."
