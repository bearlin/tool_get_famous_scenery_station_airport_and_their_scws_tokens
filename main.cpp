////////////////////////////////////////////////////////////////////////////////
// This file contains the implementation to send Taiwan/China airports/scenery/stations name strings to SCWS, 
// get output tokens in both normalized and non-normalized form, then save these results to csv files
////////////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <string>
#include <map>
#include <fstream>      // std::ofstream
#include <cstdio>
#include <cstring>
#include <cstdlib>     /* atoi */

#include "xtree.h"
#include "xdb.h"
#include "xdict.h"
#include "scws.h"

#include "TTLog.h"
DEFINE_LOGGER(gLogCXdbFilter, "CXdbFilter")

#define _CONVERT_NORMALIZE_

std::string IN_DIR = "";
std::string OUT_DIR = "";
std::string IN_PATH_SCWS_XDB = "";
std::string IN_PATH_SCWS_RULE = "";
std::string IN_PATH_NORM_MAP = "";
std::string IN_PATH_AIRPORTS = "";
std::string IN_PATH_SCENERY = "";
std::string IN_PATH_STATIONS = "";
std::string OUT_PATH_AIRPORTS = "";
std::string OUT_PATH_SCENERY = "";
std::string OUT_PATH_STATIONS = "";

#define MAX_LINE_SIZE (1024)

char tmp_buf1[MAX_LINE_SIZE];
static int iEnableHPNormalize = 0;

#ifdef _CONVERT_NORMALIZE_
// Length of multibyte character from first byte of Utf8
static const unsigned char iUTF8MultibyteLengthTable[] = 
{
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
  2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
  3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
  4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 6, 6, 1, 1
};

const unsigned int KNormBufUnitSize = 0x200;
const unsigned int KCJKBytes = 3;
std::map<std::string,std::string> iNormalizerMap;

int CHomophoneNormalizer_Init(const char* aFile)
{
  FILE* fileStage07NormalizationMap;
  char tempString[MAX_LINE_SIZE];

  fileStage07NormalizationMap = fopen(aFile, "r");
  if (fileStage07NormalizationMap == NULL)
  {
    LOG_INFO(gLogCXdbFilter, "Error fopen homophone map file: %s\n", aFile);
    return false;
  }

  int insertCount = 0;
  while (fgets(tempString, sizeof(tempString), fileStage07NormalizationMap) != NULL)
  {
    strtok(tempString, "\r\n");

    const char* pinyinCharacter;
    const char* sourceCharacter;
    const char* mappedCharacter;

    pinyinCharacter = strtok(tempString, ",");
    sourceCharacter = pinyinCharacter ? strtok(NULL, ",") : NULL;
    mappedCharacter = sourceCharacter ? strtok(NULL, ",") : NULL;

    if (pinyinCharacter && sourceCharacter && mappedCharacter)
    {
      //iNormalizerMap.Set((CHomophoneMap::TUnit) sourceCharacter, (CHomophoneMap::TUnit) mappedCharacter);
      if (iNormalizerMap.end() == iNormalizerMap.find(sourceCharacter))
      {
        iNormalizerMap.insert(std::pair<std::string,std::string>(sourceCharacter, mappedCharacter));
        ++insertCount;
      }
    }
    //LOG_DEBUG(gFtsChHPNormalizer, "iHomophoneMap[%s]=%s\n", sourceCharacter, mappedCharacter);
  }

  fclose(fileStage07NormalizationMap);
  LOG_INFO(gLogCXdbFilter, "Read Homophone mapping into map DONE, total=%d.\n", insertCount);

  return true;
}

size_t CHomophoneNormalizer_Normalize(const char* aSourceString, char* aOutputString, size_t aLength)
{
  size_t stringLength = 0;
  size_t utf8FirstByteOffset = 0;
  size_t returnLength = 0;
  const unsigned char* inputString = (const unsigned char*)aSourceString;

  //LOG_INFO(gLogCXdbFilter, "CHomophoneNormalizer_Normalize aSourceString:%s aLength:%d\n", aSourceString, aLength);
  stringLength = strlen(aSourceString);
  if ((stringLength + 1) > aLength)
  {
    returnLength = stringLength + 1; // aOutputString length equals to aSourceString
    return returnLength;
  }

  for (utf8FirstByteOffset = 0; inputString[utf8FirstByteOffset];)
  {
    size_t characterLengthInBytes = iUTF8MultibyteLengthTable[inputString[utf8FirstByteOffset]];

    if (KCJKBytes != characterLengthInBytes) /* not CJK */
    {
      memcpy(aOutputString + utf8FirstByteOffset, inputString + utf8FirstByteOffset, characterLengthInBytes);
    }
    else  /* CJK */
    {
      //if (iNormalizerMap.IsContained(inputString + utf8FirstByteOffset))
      std::string CJKStr((const char*)(inputString + utf8FirstByteOffset), characterLengthInBytes);
      if (iNormalizerMap.end() != iNormalizerMap.find(CJKStr))
      {
        memcpy(aOutputString + utf8FirstByteOffset, iNormalizerMap[CJKStr].data(), characterLengthInBytes);
      }
      else
      {
        memcpy(aOutputString + utf8FirstByteOffset, inputString + utf8FirstByteOffset, characterLengthInBytes);
      }
    }

    utf8FirstByteOffset += characterLengthInBytes;
  }

  aOutputString[utf8FirstByteOffset] = 0;
  //LOG_INFO(gLogCXdbFilter, "HPNormalized result:%s\n", aOutputString);
  returnLength = 0;
  return returnLength;
}

void CFtsTokenizerExtChinese_ReserveStringCapacity(std::string& aString, size_t aSize, size_t aUnitSize)
{
  const size_t allocateSize = (aSize + aUnitSize - 1) & ~(aUnitSize - 1); // pack with aUnitSize
  aString.resize(allocateSize);
}
#endif //_CONVERT_NORMALIZE_

bool TokenGet(std::string xdbPath, std::string rulePath, std::string inputPath, std::string outputPath)
{
  char tempToken[MAX_LINE_SIZE];
  scws_t scwsHandle;
  scws_res_t scwsResults, scwsCurrentResult;
  int status;

  char lineTextToScws[MAX_LINE_SIZE];
  char lineText[MAX_LINE_SIZE];
  int textSize;
  int lineNumber = 0;

  FILE* fileRaw;
  std::ofstream ofs;

#ifdef _CONVERT_NORMALIZE_
  char* pKey = NULL;
  std::string normalizedTokens;
  std::string iNormText;

  iNormText.resize(32);
#endif //_CONVERT_NORMALIZE_

  if (!(scwsHandle = scws_new()))
  {
    printf("ERROR: cann't init the scws!\n");
    return -1;
  }
  scws_set_charset(scwsHandle, "utf8");
  status = scws_set_dict(scwsHandle, xdbPath.c_str(), SCWS_XDICT_XDB);
  scws_set_rule(scwsHandle, rulePath.c_str() );

  printf("input src:%s\n", inputPath.c_str());
  fileRaw = fopen(inputPath.c_str(), "r");
  if (NULL == fileRaw)
  {
    printf("fileRaw err:%s\n", inputPath.c_str());
    return -1;
  }
  printf("fileRaw:%s\n", inputPath.c_str());
  
  ofs.open(outputPath.c_str(), std::ofstream::out);

  // Write cvs header
  std::cout << "InputString, Expect_Non_Normalized, Expect_Normalized\n";
  ofs << "InputString, Expect_Non_Normalized, Expect_Normalized\n";

  while (fgets(lineText, MAX_LINE_SIZE, fileRaw) != NULL)
  {
    lineNumber++;
    if ((lineNumber % 10) == 0)
    {
      LOG_INFO(gLogCXdbFilter, "Parsing Line(%d)....\n", lineNumber);
    }

    if (strlen(lineText) == 0)
    {
      continue;
    }

    textSize = 0;
    memset(lineTextToScws, 0, sizeof(lineTextToScws));

    // Copy input string to new buffer and strip newline characters
    char* ptrLine = lineText;
    while (('\r' != *ptrLine) && ('\n' != *ptrLine) && (0 != *ptrLine))
    {
      lineTextToScws[textSize++] = *(ptrLine++);
    }

    // Clear normailzed tokens buffer
    normalizedTokens.clear();

    std::cout << lineTextToScws << ",";
    ofs << lineTextToScws << ",";
    scws_send_text(scwsHandle, lineTextToScws, textSize);
    while (scwsResults = scwsCurrentResult = scws_get_result(scwsHandle))
    {
      while (scwsCurrentResult != NULL)
      {
        memset(tempToken, 0, sizeof(tempToken));
        memcpy(tempToken, &lineTextToScws[scwsCurrentResult->off], scwsCurrentResult->len);

        std::cout << tempToken << " ";
        ofs << tempToken << " ";

        // Convert this token to normalized form
      #ifdef _CONVERT_NORMALIZE_
        pKey = tempToken;
        if(iEnableHPNormalize)
        {
          size_t res_len = CHomophoneNormalizer_Normalize(pKey, &iNormText[0], iNormText.capacity());
          
          if(res_len != 0) // Buffer size is too small
          {
            //printf("@@@ Buffer size is too small, pKey length=%d,  iNormText.size()=%d,  iNormText.capacity()=%d\n", res_len, iNormText.size(), iNormText.capacity());
            CFtsTokenizerExtChinese_ReserveStringCapacity(iNormText, res_len, KNormBufUnitSize);

            if((res_len = CHomophoneNormalizer_Normalize(pKey, &iNormText[0], iNormText.capacity())))
            {
              printf("Normalize error=%ld\n", res_len);
              return 0;
            }
          }
          //printf("iNormText.c_str():%s\n", iNormText.c_str());
          //printf("strlen(iNormText.c_str()):%d\n", strlen(iNormText.c_str()));

          memset(tmp_buf1, 0, sizeof(tmp_buf1));
          //printf("AAA Un-Normali str:%s", tmp_buf1);
          strncpy(tmp_buf1, iNormText.c_str(), strlen(iNormText.c_str()));
          //printf("BBB Normalized str:%s", tmp_buf1);

          // Save this normalized token
          normalizedTokens += tmp_buf1;
          normalizedTokens += " ";
        }
      #endif //_CONVERT_NORMALIZE_

        scwsCurrentResult = scwsCurrentResult->next;
      }

      scws_free_result(scwsResults);
    }

    // Append normalizedTokens
    std::cout << ",";
    ofs << ",";

    ofs << normalizedTokens;
    std::cout << normalizedTokens;

    // Next line.
    ofs << '\n';
    std::cout << "\n";
  }
  scws_free(scwsHandle);
  printf("Total Parsing Line(%d)....\n", lineNumber);

  ofs.close();
  fclose(fileRaw);
}


int main(int argc, char* argv[])
{
  std::string iFilePath;
  std::string mapType = "taiwan";

  if (argc < 2)
  {
    std::cout << "Usage : ./scws_get_tokens [taiwan|china]" << std::endl;
    return EXIT_FAILURE;
  }
  else
  {
    mapType = argv[1];
  }

  if ( (mapType != std::string("taiwan")) && (mapType != std::string("china")) )
  {
    std::cout << "Unknow mapType: " << mapType << std::endl;
    return EXIT_FAILURE;
  }
  std::cout << "mapType: " << mapType << std::endl;

  IN_DIR = "../inputs/";
  if (mapType == std::string("taiwan"))
  {
    OUT_DIR = "../outputs/taiwan/";
    IN_PATH_SCWS_XDB = "taiwan/xdb/optimized.xdb";
    IN_PATH_SCWS_RULE = "taiwan/xdb/rules.utf8.modified.ini";
    IN_PATH_NORM_MAP = "taiwan/xdb/normalize.mapping.table.ini";
    IN_PATH_AIRPORTS = "airports/taiwan_airports_raw.txt.dump";
    IN_PATH_SCENERY = "scenery/taiwan_scenery_raw.txt.dump";
    IN_PATH_STATIONS = "stations/taiwan_stations_raw.txt.dump";
    OUT_PATH_AIRPORTS = "taiwan_airports.csv";
    OUT_PATH_SCENERY = "taiwan_scenery.csv";
    OUT_PATH_STATIONS = "taiwan_stations.csv";
  }
  else
  {
    OUT_DIR = "../outputs/china/";
    IN_PATH_SCWS_XDB = "china/xdb/optimized.xdb";
    IN_PATH_SCWS_RULE = "china/xdb/rules.utf8.modified.ini";
    IN_PATH_NORM_MAP = "china/xdb/normalize.mapping.table.ini";
    IN_PATH_AIRPORTS = "airports/china_airports_raw.txt.dump";
    IN_PATH_SCENERY = "scenery/china_scenery_raw.txt.dump";
    IN_PATH_STATIONS = "stations/china_stations_raw.txt.dump";
    OUT_PATH_AIRPORTS = "china_airports.csv";
    OUT_PATH_SCENERY = "china_scenery.csv";
    OUT_PATH_STATIONS = "china_stations.csv";
  }

#ifdef _CONVERT_NORMALIZE_
  iFilePath = std::string(IN_DIR) + IN_PATH_NORM_MAP;
  if (!CHomophoneNormalizer_Init(iFilePath.c_str()))
  {
    LOG_INFO(gLogCXdbFilter, "CHomophoneNormalizer_Init err:%s\n", iFilePath.c_str());
    iEnableHPNormalize = 0;
  }
  else
  {
    LOG_INFO(gLogCXdbFilter, "CHomophoneNormalizer_Init OK\n");
    iEnableHPNormalize = 1;
  }
#endif //_CONVERT_NORMALIZE_

  TokenGet(std::string(IN_DIR) + IN_PATH_SCWS_XDB, std::string(IN_DIR) + IN_PATH_SCWS_RULE, std::string(IN_DIR) + IN_PATH_AIRPORTS, std::string(OUT_DIR) + OUT_PATH_AIRPORTS);
  TokenGet(std::string(IN_DIR) + IN_PATH_SCWS_XDB, std::string(IN_DIR) + IN_PATH_SCWS_RULE, std::string(IN_DIR) + IN_PATH_SCENERY, std::string(OUT_DIR) + OUT_PATH_SCENERY);
  TokenGet(std::string(IN_DIR) + IN_PATH_SCWS_XDB, std::string(IN_DIR) + IN_PATH_SCWS_RULE, std::string(IN_DIR) + IN_PATH_STATIONS, std::string(OUT_DIR) + OUT_PATH_STATIONS);

  return EXIT_SUCCESS;
}

