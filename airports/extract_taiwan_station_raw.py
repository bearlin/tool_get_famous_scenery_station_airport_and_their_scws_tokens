#!/usr/bin/python

from sys import argv
import codecs

print "Start."
script, filename = argv
print "script: %r." % script
print "filename: %r." % filename

print "Opening the rawfile..."
rawfile = codecs.open(filename, 'r', encoding='utf-8')
dumpfile = codecs.open(filename + ".dump", 'w', encoding='utf-8')

# Processing matched strings
lineCount=0
resultList = []
for string in rawfile:
  lineCount = lineCount + 1
  print "Processing[%d]:%s" % (lineCount, string)

  if (lineCount < 3) :
    continue

  strtmp1 = string

  # Get first token of this line
  keyIdxNext = strtmp1.find('\t', 0)
  strtmp2 = strtmp1[0:keyIdxNext]
  print "Found token:%s" % (strtmp2)
  strtmp2.lstrip().rstrip()
  resultList.append(strtmp2);

  if (keyIdxNext != -1):
    startIdx = keyIdxNext + 1
    keyIdxEnd = strtmp1.find('\t', startIdx)
    endIdx = keyIdxEnd
    # print "keyIdxNext01:%r" % (keyIdxNext)
    # print "keyIdxEnd:%r" % (keyIdxEnd)
    # print "startIdx:%r" % (startIdx)
    # print "endIdx:%r" % (endIdx)
    strtmp2 = strtmp1[startIdx:endIdx]
    print "Found token:%s" % (strtmp2)

    # Put to a list
    strtmp2.lstrip().rstrip()
    resultList.append(strtmp2);


for wstr in set(resultList):
# resultList.sort()
# for wstr in resultList:
  # Write stripped result to dumpfile
  dumpfile.write(wstr)
  dumpfile.write("\n")

dumpfile.close()
rawfile.close()

print "Done."
