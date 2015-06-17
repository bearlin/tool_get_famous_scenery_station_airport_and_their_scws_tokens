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
pattern = re.compile("<td\ align=\"left\">.*title=\".*</a></td>")
strings = re.findall(pattern, rawfile.read())
print "%r" % (strings)

# Processing matched strings
lineCount=0
resultList = []
for string in strings:
  lineCount = lineCount + 1
  print "Processing[%d]:%s" % (lineCount, string)

  strtmp1 = string
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
    if (strtmp2.find('\"') != -1) :
      continue

    # Put to a list
    strtmp2.lstrip().rstrip()
    resultList.append(strtmp2);

  #if (lineCount == 1) :
  #  break

for wstr in set(resultList):
# resultList.sort()
# for wstr in resultList:
  # Write stripped result to dumpfile
  dumpfile.write(wstr)
  dumpfile.write("\n")

dumpfile.close()
rawfile.close()

print "Done."
