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

#define USE_TC
//#define _CONVERT_NORMALIZE_

#define MAX_LINE_SIZE (1024)

#ifdef USE_TC
#define IN_DIR "../inputs/"
#define OUT_DIR "../outputs/taiwan/"
#define IN_PATH_SCWS_XDB"taiwan/xdb/dict_cht.utf8.xdb"
#define IN_PATH_SCWS_RULE"taiwan/xdb/fts-tc-r.tok"
#define IN_PATH_S07_NORM_MAP "taiwan/xdb/fts-tc-n.tok"
#define IN_PATH_AIRPORTS "airports/taiwan_airports_raw.txt.dump"
#define IN_PATH_SCENERY "scenery/taiwan_scenery_raw.txt.dump"
#define IN_PATH_STATIONS "stations/taiwan_stations_raw.txt.dump"
#define OUT_PATH_AIRPORTS "taiwan_airports_raw.txt.dump"
#define OUT_PATH_SCENERY "taiwan_scenery_raw.txt.dump"
#define OUT_PATH_STATIONS "taiwan_stations_raw.txt.dump"
#else //USE_TC
#define IN_DIR "../inputs/"
#define OUT_DIR "../outputs/china/"
#define IN_PATH_SCWS_XDB"china/xdb/dict.utf8.xdb"
#define IN_PATH_SCWS_RULE"china/xdb/fts-sc-r.tok"
#define IN_PATH_NORM_MAP "china/xdb/fts-sc-n.tok"
#define IN_PATH_AIRPORTS "airports/china_airports_raw.txt.dump"
#define IN_PATH_SCENERY "scenery/china_scenery_raw.txt.dump"
#define IN_PATH_STATIONS "stations/china_stations_raw.txt.dump"
#define OUT_PATH_AIRPORTS "china_airports_raw.txt.dump"
#define OUT_PATH_SCENERY "china_scenery_raw.txt.dump"
#define OUT_PATH_STATIONS "china_stations_raw.txt.dump"
#endif //USE_TC

char tmp_buf1[MAX_LINE_SIZE];

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

int main(int argc, char* argv[])
{
  std::string file_path;
  FILE *fp_xdb;
  char tmp_buf[MAX_LINE_SIZE];
  std::string token_item;
  int token_idx = 0;
  
  int line_no=0;
  char* pline=0;

  std::string tmp_line;

  std::size_t pos;
  std::string str1;
  std::string str2;
  int idx_tmp = 0;
  int idx_max = 0;

  size_t readSize = 0;

  printf("Stage 1: Collect token list from \"input raw file\" to token_map.\n");
  char szTmp[MAX_LINE_SIZE];
  scws_t s;
  scws_res_t res, cur;
  int ret;
  char text[MAX_LINE_SIZE];
  char line_text[MAX_LINE_SIZE];
  int text_size;

  if (!(s = scws_new()))
  {
    printf("ERROR: cann't init the scws!\n");
    return -1;
  }

  scws_set_charset(s, "utf8");

  file_path = IN_DIR;
  file_path += IN_PATH_SCWS_XDB;
  ret = scws_set_dict(s, file_path.c_str(), SCWS_XDICT_XDB);

  file_path = IN_DIR;
  file_path += IN_PATH_SCWS_RULE;
  scws_set_rule(s, file_path.c_str() );

  FILE *fp_s01_raw;
  file_path = IN_DIR;
  file_path += IN_PATH_AIRPORTS;
  printf("input src:%s\n", file_path.c_str());
  fp_s01_raw = fopen(file_path.c_str(), "r");
  if (NULL == fp_s01_raw)
  {
    printf("fp_s01_raw err:%s\n", file_path.c_str());
    return -1;
  }
  printf("fp_s01_raw:%s\n", file_path.c_str());
  
  std::ofstream ofs;
  file_path = OUT_DIR;
  file_path += OUT_PATH_AIRPORTS;
  ofs.open(file_path.c_str(), std::ofstream::out);
  while( NULL != fgets(line_text, MAX_LINE_SIZE, fp_s01_raw) ) 
  {
    line_no++;
    if (0 == line_no%10)
      printf("Parsing Line(%d)....\n", line_no );
    
    if( 0 == strlen(line_text) )
      continue;

    text_size = 0;
    memset(text,0, sizeof(text) );

    pline = line_text;
    while( ('\r' != *pline) && ( '\n' != *pline) && ( 0 != *pline) )
      text[text_size++] = *(pline++);

    printf("%s\n", text);
    scws_send_text(s, text, text_size);
    while (res = cur = scws_get_result(s))
    {
      while (cur != NULL)
      {
        memset(szTmp, 0, sizeof(szTmp));
        memcpy(szTmp, &text[cur->off], cur->len);

        ofs << szTmp << '\n';
        std::cout << szTmp << '\n';

        cur = cur->next;
      }

      scws_free_result(res);
    }
  }
  scws_free(s);
  printf("Total Parsing Line(%d)....\n", line_no );

  ofs.close();
  fclose(fp_s01_raw);
  //------------------------------------------------------------------------------------------------

#ifdef _CONVERT_NORMALIZE_
  // Stage 7: Convert results to normalized form.
  //------------------------------------------------------------------------------------------------
#if 1
  printf("Stage 7: Convert results to normalized form.\n");
  FILE *fp_in_s07_optimized_full;
  FILE *fp_out_s07_normalized_full;
  int szLine_Len;
  char* pKey=0;

  int iEnableHPNormalize = 0;
  std::string iNormText;

  file_path = DATA_DIR;
  file_path += IN_PATH_S07_NORM_MAP;
  if (!CHomophoneNormalizer_Init(file_path.c_str()))
  {
    printf("CHomophoneNormalizer_Init err:%s\n", file_path.c_str());
    iEnableHPNormalize = 0;
  }
  else
  {
    printf("CHomophoneNormalizer_Init OK\n");
    iEnableHPNormalize = 1;
  }
  
  file_path = DATA_DIR;
  file_path += OUT_PATH_S06_SUFFIX_FULL;
  fp_in_s07_optimized_full = fopen(file_path.c_str(), "r");
  if (NULL == fp_in_s07_optimized_full)
  {
    printf("fp_in_s07_optimized_full err:%s\n", file_path.c_str());
    return -1;
  }
  printf("fp_in_s07_optimized_full:%s\n", file_path.c_str());

  file_path = DATA_DIR;
  file_path += OUT_PATH_S07_SUFFIX_FULL_NOR;
  fp_out_s07_normalized_full = fopen(file_path.c_str(), "w");
  if (NULL == fp_out_s07_normalized_full)
  {
    printf("fp_out_s07_normalized_full err:%s\n", file_path.c_str());
    return -1;
  }
  printf("fp_out_s07_normalized_full:%s\n", file_path.c_str());

  // Start converting.
  iNormText.resize(32);
  while (fgets(tmp_buf1, sizeof(tmp_buf1), fp_in_s07_optimized_full)) 
  {
    line_no++;
    if (0 == line_no%1000)
      printf("Converting Line(%d)....\n", line_no );

    if ((tmp_buf1[0] == '#') || (tmp_buf1[0] == ';'))
      continue;

    strncpy(tmp_buf, tmp_buf1, sizeof(tmp_buf));
    szLine_Len = strlen(tmp_buf);

    if( 0 == szLine_Len )
      continue;

    pKey = strtok(tmp_buf, "\t ");
    if (pKey) 
    {
      //printf("Converting [%s] of %s", pKey, tmp_buf1 );

      // Normalizer
      //iNormText.resize(32);
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

        //printf("AAA Un-Normali str:%s", tmp_buf1);
        strncpy(tmp_buf1, iNormText.c_str(), strlen(iNormText.c_str()));
        //printf("BBB Normalized str:%s", tmp_buf1);

        //log to file.
        fprintf(fp_out_s07_normalized_full, "%s", tmp_buf1);
      }
    }
    else
    {
      printf("Failed converting:%s", tmp_buf1 );
    }
  }
  printf("Total converting Line(%d)....\n", line_no );

  fclose(fp_in_s07_optimized_full);
  fclose(fp_out_s07_normalized_full);
#endif
  //------------------------------------------------------------------------------------------------
#endif //_CONVERT_NORMALIZE_

  return 0;
}

