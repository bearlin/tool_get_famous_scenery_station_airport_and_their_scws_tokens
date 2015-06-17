// xdb_filter.cpp : Defines the entry point for the console application.
//

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

/* #define USE_TC */
#define _CONVERT_NORMALIZE_

/* #ifdef USE_TC */
/* #define IN_DIR "../inputs/" */
/* #define OUT_DIR "../outputs/taiwan/" */
/* #define IN_PATH_SCWS_XDB "taiwan/xdb/dict_cht.utf8.xdb" */
/* #define IN_PATH_SCWS_RULE "taiwan/xdb/fts-tc-r.tok" */
/* #define IN_PATH_NORM_MAP "taiwan/xdb/fts-tc-n.tok" */
/* #define IN_PATH_AIRPORTS "airports/taiwan_airports_raw.txt.dump" */
/* #define IN_PATH_SCENERY "scenery/taiwan_scenery_raw.txt.dump" */
/* #define IN_PATH_STATIONS "stations/taiwan_stations_raw.txt.dump" */
/* #define OUT_PATH_AIRPORTS "taiwan_airports.csv" */
/* #define OUT_PATH_SCENERY "taiwan_scenery.csv" */
/* #define OUT_PATH_STATIONS "taiwan_stations.csv" */
/* #else //USE_TC */
/* #define IN_DIR "../inputs/" */
/* #define OUT_DIR "../outputs/china/" */
/* #define IN_PATH_SCWS_XDB"china/xdb/dict.utf8.xdb" */
/* #define IN_PATH_SCWS_RULE"china/xdb/fts-sc-r.tok" */
/* #define IN_PATH_NORM_MAP "china/xdb/fts-sc-n.tok" */
/* #define IN_PATH_AIRPORTS "airports/china_airports_raw.txt.dump" */
/* #define IN_PATH_SCENERY "scenery/china_scenery_raw.txt.dump" */
/* #define IN_PATH_STATIONS "stations/china_stations_raw.txt.dump" */
/* #define OUT_PATH_AIRPORTS "china_airports.csv" */
/* #define OUT_PATH_SCENERY "china_scenery.csv" */
/* #define OUT_PATH_STATIONS "china_stations.csv" */
/* #endif //USE_TC */

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
static const unsigned char g_mblen_table_utf8[] = 
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

std::map<std::string,std::string> iMap;
std::map<std::string,std::string>::iterator iMap_it;

int CHomophoneNormalizer_Init(const char* aFile)
{
  FILE* fd;
  char  line[256];
  
  fd = fopen(aFile, "r");
  if(NULL == fd)
  {
    printf("Error fopen homophone map file: %s\n", aFile);
    return 0;
  }

  int ct = 0;
  while(fgets(line, sizeof(line), fd)) 
  {
    strtok(line, "\r\n");
  
    const char *pinyin, *src, *dst;

    pinyin = strtok(line, ",");
    src    = pinyin ? strtok(NULL, ",") : NULL;
    dst    = src ?    strtok(NULL, ",") : NULL;

    if(pinyin && src && dst)
    {
      //iMap.Set((CHomophoneMap::TUnit) src, (CHomophoneMap::TUnit) dst);
      if (iMap.end() == iMap.find(src))
      {
        iMap.insert( std::pair<std::string,std::string>(src,dst) );
        ++ct;
      }
    }
    
    //LOG_DEBUG(gFtsChHPNormalizer, "iHomophoneMap[%s]=%s\n", src, dst);
  }
  
  fclose(fd);

  printf("Read Homophone mapping into map DONE, total=%d.\n", ct);

  return 1;
}

size_t CHomophoneNormalizer_Normalize(const char* aSource, char* aOutput, size_t aLength)
{
  size_t i;
  const unsigned char* aInput = (const unsigned char*) aSource;

  //printf("CHomophoneNormalizer_Normalize aSource:%s aLength:%d\n", aSource, aLength);
  
  if((i = strlen(aSource)) + 1 > aLength)
  {
    return i + 1; // aOutput length equals to aSource
  }

  for(i = 0; aInput[i];)
  {
    size_t lenMB = g_mblen_table_utf8[aInput[i]];

    if(KCJKBytes != lenMB) /* not CJK */
    {
      memcpy(aOutput + i, aInput + i, lenMB);
    }
    else  /* CJK */
    {
      //if(iMap.IsContained(aInput+i)) 
      std::string CJKStr((const char*)(aInput+i), lenMB);
      if (iMap.end() != iMap.find(CJKStr))
      {
        memcpy(aOutput + i, iMap[CJKStr].data(), lenMB);
      }
      else 
      {
        memcpy(aOutput + i, aInput + i, lenMB);
      }
    }

    i += lenMB;
  }

  aOutput[i] = 0;

  //printf("HPNormalized result:%s\n", aOutput);

  return 0;
}

void CFtsTokenizerExtChinese_ReserveStringCapacity(std::string& aString, size_t aSize, size_t aUnitSize)
{
  const size_t allocSize = (aSize + aUnitSize - 1) & ~(aUnitSize - 1); // pack with aUnitSize
  aString.resize(allocSize);
}
#endif //_CONVERT_NORMALIZE_

bool TokenGet(std::string xdbPath, std::string rulePath, std::string inputPath, std::string outputPath)
{
  int line_no=0;
  char* pline=0;

  char szTmp[MAX_LINE_SIZE];
  scws_t s;
  scws_res_t res, cur;
  int ret;
  char text[MAX_LINE_SIZE];
  char line_text[MAX_LINE_SIZE];
  int text_size;

  FILE* fp_input;
  std::ofstream ofs;

#ifdef _CONVERT_NORMALIZE_
  char* pKey = NULL;
  std::string normalizedTokens;
  std::string iNormText;

  iNormText.resize(32);
#endif //_CONVERT_NORMALIZE_

  if (!(s = scws_new()))
  {
    printf("ERROR: cann't init the scws!\n");
    return -1;
  }
  scws_set_charset(s, "utf8");
  ret = scws_set_dict(s, xdbPath.c_str(), SCWS_XDICT_XDB);
  scws_set_rule(s, rulePath.c_str() );

  printf("input src:%s\n", inputPath.c_str());
  fp_input = fopen(inputPath.c_str(), "r");
  if (NULL == fp_input)
  {
    printf("fp_input err:%s\n", inputPath.c_str());
    return -1;
  }
  printf("fp_input:%s\n", inputPath.c_str());
  
  ofs.open(outputPath.c_str(), std::ofstream::out);

  // Write cvs header
  std::cout << "InputString, Expect_Non_Normalized, Expect_Normalized\n";
  ofs << "InputString, Expect_Non_Normalized, Expect_Normalized\n";

  while( NULL != fgets(line_text, MAX_LINE_SIZE, fp_input) ) 
  {
    line_no++;
    if (0 == line_no%10)
      printf("Parsing Line(%d)....\n", line_no );
    
    if( 0 == strlen(line_text) )
      continue;

    text_size = 0;
    memset(text,0, sizeof(text) );

    // Copy input string to new buffer and strip newline characters
    pline = line_text;
    while( ('\r' != *pline) && ( '\n' != *pline) && ( 0 != *pline) )
      text[text_size++] = *(pline++);

    // Clear normailzed tokens buffer
    normalizedTokens.clear();

    std::cout << text << ",";
    ofs << text << ",";
    scws_send_text(s, text, text_size);
    while (res = cur = scws_get_result(s))
    {
      while (cur != NULL)
      {
        memset(szTmp, 0, sizeof(szTmp));
        memcpy(szTmp, &text[cur->off], cur->len);

        std::cout << szTmp << " ";
        ofs << szTmp << " ";

        // Convert this token to normalized form
      #ifdef _CONVERT_NORMALIZE_
        pKey = szTmp;
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

        cur = cur->next;
      }

      scws_free_result(res);
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
  scws_free(s);
  printf("Total Parsing Line(%d)....\n", line_no );

  ofs.close();
  fclose(fp_input);
}


int main(int argc, char* argv[])
{
  std::string file_path_norm;
  std::string mapType = "taiwan";

  if (argc < 2)
  {
    std::cout << "Usage : ./scws_get_tokens [taiwan|china]" << std::endl;
    return 0;
  }
  else
  {
    mapType = argv[1];
  }

  if ( (mapType != std::string("taiwan")) && (mapType != std::string("china")) )
  {
    std::cout << "Unknow mapType: " << mapType << std::endl;
    return 0;
  }
  std::cout << "mapType: " << mapType << std::endl;

  if (mapType == std::string("taiwan"))
  {
    IN_DIR = "../inputs/";
    OUT_DIR = "../outputs/taiwan/";
    IN_PATH_SCWS_XDB = "taiwan/xdb/dict_cht.utf8.xdb";
    IN_PATH_SCWS_RULE = "taiwan/xdb/fts-tc-r.tok";
    IN_PATH_NORM_MAP = "taiwan/xdb/fts-tc-n.tok";
    IN_PATH_AIRPORTS = "airports/taiwan_airports_raw.txt.dump";
    IN_PATH_SCENERY = "scenery/taiwan_scenery_raw.txt.dump";
    IN_PATH_STATIONS = "stations/taiwan_stations_raw.txt.dump";
    OUT_PATH_AIRPORTS = "taiwan_airports.csv";
    OUT_PATH_SCENERY = "taiwan_scenery.csv";
    OUT_PATH_STATIONS = "taiwan_stations.csv";
  }
  else
  {
    IN_DIR = "../inputs/";
    OUT_DIR = "../outputs/china/";
    IN_PATH_SCWS_XDB = "china/xdb/dict.utf8.xdb";
    IN_PATH_SCWS_RULE = "china/xdb/fts-sc-r.tok";
    IN_PATH_NORM_MAP = "china/xdb/fts-sc-n.tok";
    IN_PATH_AIRPORTS = "airports/china_airports_raw.txt.dump";
    IN_PATH_SCENERY = "scenery/china_scenery_raw.txt.dump";
    IN_PATH_STATIONS = "stations/china_stations_raw.txt.dump";
    OUT_PATH_AIRPORTS = "china_airports.csv";
    OUT_PATH_SCENERY = "china_scenery.csv";
    OUT_PATH_STATIONS = "china_stations.csv";
  }

#ifdef _CONVERT_NORMALIZE_
  file_path_norm = std::string(IN_DIR) + IN_PATH_NORM_MAP;
  if (!CHomophoneNormalizer_Init(file_path_norm.c_str()))
  {
    printf("CHomophoneNormalizer_Init err:%s\n", file_path_norm.c_str());
    iEnableHPNormalize = 0;
  }
  else
  {
    printf("CHomophoneNormalizer_Init OK\n");
    iEnableHPNormalize = 1;
  }
#endif //_CONVERT_NORMALIZE_

  TokenGet(std::string(IN_DIR) + IN_PATH_SCWS_XDB, std::string(IN_DIR) + IN_PATH_SCWS_RULE, std::string(IN_DIR) + IN_PATH_AIRPORTS, std::string(OUT_DIR) + OUT_PATH_AIRPORTS);
  TokenGet(std::string(IN_DIR) + IN_PATH_SCWS_XDB, std::string(IN_DIR) + IN_PATH_SCWS_RULE, std::string(IN_DIR) + IN_PATH_SCENERY, std::string(OUT_DIR) + OUT_PATH_SCENERY);
  TokenGet(std::string(IN_DIR) + IN_PATH_SCWS_XDB, std::string(IN_DIR) + IN_PATH_SCWS_RULE, std::string(IN_DIR) + IN_PATH_STATIONS, std::string(OUT_DIR) + OUT_PATH_STATIONS);

  return 0;
}

