#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "oceanbase.h"

const int MAX_DISPLAY_ROW_KEY_LEN = 48;

int64_t microseconds() {
    struct timeval tv; 
      gettimeofday(&tv, NULL);
        return tv.tv_sec * 1000000L + tv.tv_usec;
}

char* get_display_row_key(const char* row_key, int64_t row_key_len)
{
  const int aux_len = 2;
  const int ending_len = 1;
  static char dis_row_key[MAX_DISPLAY_ROW_KEY_LEN + aux_len + ending_len];
  int64_t dis_row_key_len = row_key_len <= MAX_DISPLAY_ROW_KEY_LEN / 2 ?
                            row_key_len : MAX_DISPLAY_ROW_KEY_LEN / 2;
  int64_t dis_len = dis_row_key_len * 2;
  dis_row_key[0] = '0';
  dis_row_key[1] = 'x';
  int i = 0;
  for (i = 0; i < dis_row_key_len; i++)
  {
    snprintf(dis_row_key + i * 2 + aux_len, 3, "%02x", (unsigned char)row_key[i]);
  }
  dis_row_key[dis_len + aux_len] = '\0';
  return dis_row_key;
}

char* rowkey2hex(const char* row_key, int64_t row_key_len)
{
  static char* hex = NULL;
  static int64_t hex_len = 0;
  if (NULL == hex || hex_len <= row_key_len)
  {
    if (NULL != hex && hex_len <= row_key_len)
    {
      free(hex);
      hex = NULL;
    }
    hex = (char*)malloc(row_key_len + 1);
    if (NULL != hex)
    {
      hex_len = row_key_len + 1;
    }
  }
  if (NULL != hex)
  {
    snprintf(hex, hex_len, "%.*x", row_key_len, row_key);
  }

  return hex;
}

char* trim_newline(char* s)
{
  int s_len = strlen(s);
  while (s_len > 0 && s[s_len - 1] == '\n')
  {
    s[s_len - 1] = '\0';
    s_len--;
  }
  return s;
}

void pcell(OB_CELL* cell)
{
  printf("%ld", cell->table_len);
  printf("%.*s\t%s(%ld)\t%.*s\t",
         (int)cell->table_len, cell->table,
         rowkey2hex(cell->row_key, cell->row_key_len), cell->row_key_len,
         (int)cell->column_len, cell->column);
  switch (cell->v.type)
  {
    case OB_INT_TYPE:
      printf("%ld", cell->v.v.v_int);
      break;
    case OB_VARCHAR_TYPE:
      printf("%.*s", cell->v.v.v_varchar.len, cell->v.v.v_varchar.p);
      break;
    case OB_DATETIME_TYPE:
      printf("\t%s(%d.%d)", trim_newline(ctime(&(cell->v.v.v_datetime.tv_sec))),
          cell->v.v.v_datetime.tv_sec, cell->v.v.v_datetime.tv_usec);
      break;
  }
  if (cell->is_null || cell->is_row_not_exist || cell->is_row_changed)
  {
    printf("(%s%s%s)\n",
           cell->is_null ? "NULL " : "",
           cell->is_row_not_exist ? "ROW NOT EXIST " : "",
         cell->is_row_changed ? "NEW ROW" : "");
  }
  else
  {
    printf("\n");
  }
}

void pcolumnname(OB_RES* res)
{
  OB_CELL* cell = NULL;
  cell = ob_fetch_cell(res);
  if (NULL != cell)
  {
    printf("\t\t%.*s", cell->column_len, cell->column);
  }
  while ((cell = ob_fetch_cell(res)) != NULL && !cell->is_row_changed)
  {
    printf("\t%.*s", cell->column_len, cell->column);
  }
}

void pres(OB_RES* res)
{
  pcolumnname(res);
  printf("\n");
  ob_res_seek_to_begin_cell(res);

  OB_ROW* row = NULL;
  while ((row = ob_fetch_row(res)) != NULL)
  {
    int64_t i = 0;
    printf("%s", get_display_row_key(row->cell[0].row_key, row->cell[0].row_key_len));
    for (; i < row->cell_num; i++)
    {
      switch (row->cell[i].v.type)
      {
        case OB_INT_TYPE:
          printf("\t%ld", row->cell[i].v.v.v_int);
          break;
        case OB_VARCHAR_TYPE:
          printf("\t%.*s", row->cell[i].v.v.v_varchar.len, row->cell[i].v.v.v_varchar.p);
          break;
        case OB_DATETIME_TYPE:
          printf("\t%s(%d.%d)", trim_newline(ctime(&(row->cell[i].v.v.v_datetime.tv_sec))),
              row->cell[i].v.v.v_datetime.tv_sec, row->cell[i].v.v.v_datetime.tv_usec);
          break;
      }
    }
    printf("\n");
  }
//  OB_CELL* cell = NULL;
//  while ((cell = ob_fetch_cell(res)) != NULL)
//  {
//    if (cell->is_row_changed)
//    {
//      printf("\n");
//      printf("%s", get_display_row_key(cell->row_key, cell->row_key_len));
//    }
//    switch (cell->v.type)
//    {
//      case OB_INT_TYPE:
//        printf("\t%ld", cell->v.v.v_int);
//        break;
//      case OB_VARCHAR_TYPE:
//        printf("\t%.*s", cell->v.v.v_varchar.len, cell->v.v.v_varchar.p);
//        break;
//      case OB_DATETIME_TYPE:
//        printf("\t%d.%d", cell->v.v.v_datetime.tv_sec, cell->v.v.v_datetime.tv_usec);
//        break;
//    }
//  }
}

void set_scan(OB_SCAN* scan_st)
{
  //char table[100] = "lz_rpt_p4p_campaign_name";
  //char* table = "lz_rpt_p4p_effect_info";
  //char* table = "collect_item";
  //ob_scan(scan_st, table, NULL, 0, 0, NULL, 0, 0);
  //ob_scan(scan_st, table, NULL, 0, 0, "AAAAAAAAF", 9, 1);
  //ob_scan(scan_st, table, "AAAAAAAAA", 9, 1, NULL, 0, 0);
  //ob_scan(scan_st, table, "AAAAAAAAA", 9, 1, "AAAAAAAAF", 9, 1);
}

int main()
{
  int64_t num = 0;

  char* table = "dpunit_sr_auction_info_d";

  char rowkey1[24];
  char rowkey2[24];
  memset(rowkey1, 0x00, sizeof(rowkey1));
  memset(rowkey2, 0xFF, sizeof(rowkey2));
  memset(rowkey2, 0x00, 8);

  rowkey1[6] = 0xD6;
  rowkey1[7] = 0x40;
  rowkey2[6] = 0xD6;
  rowkey2[7] = 0x40;
  rowkey1[9] = 0x04;
  rowkey1[10] = 0xA9;
  rowkey1[11] = 0x5F;
  rowkey1[12] = 0x9B;
  rowkey1[13] = 0x57;
  rowkey2[9] = 0x04;
  rowkey2[10] = 0xAB;
  rowkey2[11] = 0xBB;
  rowkey2[12] = 0x1A;
  rowkey2[13] = 0x94;
  rowkey2[14] = 0x40;

  //000000000000D6400004A95F9B5700000000000000000000
  //000000000000D6400004ABBB1A944000FFFFFFFFFFFFFFFF

  // 1 tablets
  //rowkey2[6] = 0x09;
  //rowkey2[7] = 0x86;

  // 2 tablets
  //rowkey2[6] = 0x12;
  //rowkey2[7] = 0xF6;

  // 4 tablets
  //rowkey2[6] = 0x27;
  //rowkey2[7] = 0x05;

  // 8 tablets
  //rowkey2[6] = 0x52;
  //rowkey2[7] = 0x09;

  // 9 tablets
  //rowkey2[6] = 0x5E;
  //rowkey2[7] = 0x53;

  // 10 tablets
  //rowkey2[6] = 0x6A;
  //rowkey2[7] = 0xE7;

  // 11 tablets
  //rowkey2[6] = 0x77;
  //rowkey2[7] = 0x70;

  // 12 tablets
  //rowkey2[6] = 0x84;
  //rowkey2[7] = 0x0C;

  // 13 tablets
  //rowkey2[6] = 0x92;
  //rowkey2[7] = 0x86;

  // 14 tablets
  //rowkey2[6] = 0xA0;
  //rowkey2[7] = 0x40;

  // 15 tablets
  //rowkey2[6] = 0xB0;
  //rowkey2[7] = 0x22;

  // 16 tablets
  //rowkey2[6] = 0xC1;
  //rowkey2[7] = 0x98;

  // 17 tablets
  //rowkey2[6] = 0xD2;
  //rowkey2[7] = 0xF8;

  // 32 tablets
  //rowkey2[5] = 0x01;
  //rowkey2[6] = 0xE2;
  //rowkey2[7] = 0x16;

  OB* ob = ob_api_init();
  if (NULL == ob)
  {
    fprintf(stderr, "ob_init error: %s\n", ob_error());
    return 1;
  }

  ob_api_debug_log(ob, "INFO", NULL);
  ob_api_cntl(ob, OB_S_TIMEOUT, 100000000);

  // 连接OceanBase
  OB_ERR_CODE err = ob_connect(ob, "10.232.36.189", 12010, NULL, NULL);
  if (OB_ERR_SUCCESS != err)
  {
    fprintf(stderr, "ob_connect error: %s\n", ob_error());
  }
  else
  {
    // 获得OB_SCAN结构体
    OB_SCAN* scan_st = ob_acquire_scan_st(ob);
    if (NULL == scan_st)
    {
      fprintf(stderr, "ob_acquire_scan_st error: %s\n", ob_error());
    }
    else
    {
      ob_scan(scan_st, table, rowkey1, 24, 1, rowkey2, 24, 1);

      //ob_scan_column(scan_st, "day", 1);
      ob_scan_column(scan_st, "aucid", 1);
      //ob_scan_column(scan_st, "promotion", 1);
      ob_scan_column(scan_st, "unit_id", 1);
      //ob_scan_column(scan_st, "shopid", 1);
      //ob_scan_column(scan_st, "ipv", 1);
      //ob_scan_column(scan_st, "iuv", 1);
      //ob_scan_column(scan_st, "coll_num", 1);
      //ob_scan_column(scan_st, "gmv_trade_num", 1);
      //ob_scan_column(scan_st, "gmv_auction_num", 1);
      //ob_scan_column(scan_st, "gmv_trade_amt", 1);
      //ob_scan_column(scan_st, "gmv_winner_num", 1);
      //ob_scan_column(scan_st, "alipay_trade_num", 1);
      //ob_scan_column(scan_st, "alipay_auction_num", 1);
      //ob_scan_column(scan_st, "alipay_trade_amt", 1);
      //ob_scan_column(scan_st, "alipay_winner_num", 1);
      //ob_scan_column(scan_st, "today_trade_num", 1);
      //ob_scan_column(scan_st, "today_trade_amt", 1);
      //ob_scan_column(scan_st, "today_auction_num", 1);
      //ob_scan_column(scan_st, "plot_gmv_trade_num", 1);
      //ob_scan_column(scan_st, "plot_gmv_trade_amt", 1);
      //ob_scan_column(scan_st, "plot_gmv_auction_num", 1);
      //ob_scan_column(scan_st, "plot_gmv_winner_num", 1);
      //ob_scan_column(scan_st, "plot_alipay_trade_num", 1);
      //ob_scan_column(scan_st, "plot_alipay_trade_amt", 1);
      //ob_scan_column(scan_st, "plot_alipay_auction_num", 1);
      //ob_scan_column(scan_st, "plot_alipay_winner_num", 1);
      //ob_scan_column(scan_st, "noplot_gmv_trade_num", 1);
      //ob_scan_column(scan_st, "noplot_gmv_trade_amt", 1);
      //ob_scan_column(scan_st, "noplot_gmv_auction_num", 1);
      //ob_scan_column(scan_st, "noplot_gmv_winner_num", 1);
      //ob_scan_column(scan_st, "noplot_alipay_trade_num", 1);
      //ob_scan_column(scan_st, "noplot_alipay_trade_amt", 1);
      //ob_scan_column(scan_st, "noplot_alipay_auction_num", 1);
      //ob_scan_column(scan_st, "noplot_alipay_winner_num", 1);
      //ob_scan_column(scan_st, "plot_type", 1);

      //ob_scan_complex_column(scan_st, "day/2", "day/2", 1);
      //ob_scan_complex_column(scan_st, "aucid/2", "aucid/2", 1);
      //ob_scan_complex_column(scan_st, "promotion/2", "promotion/2", 1);
      //ob_scan_complex_column(scan_st, "unit_id/2", "unit_id/2", 1);
      //ob_scan_complex_column(scan_st, "shopid/2", "shopid/2", 1);
      //ob_scan_complex_column(scan_st, "ipv/2", "ipv/2", 1);
      //ob_scan_complex_column(scan_st, "iuv/2", "iuv/2", 1);
      //ob_scan_complex_column(scan_st, "coll_num/2", "coll_num/2", 1);
      //ob_scan_complex_column(scan_st, "gmv_trade_num/2", "gmv_trade_num/2", 1);
      //ob_scan_complex_column(scan_st, "gmv_auction_num/2", "gmv_auction_num/2", 1);
      //ob_scan_complex_column(scan_st, "gmv_trade_amt/2", "gmv_trade_amt/2", 1);
      //ob_scan_complex_column(scan_st, "gmv_winner_num/2", "gmv_winner_num/2", 1);
      //ob_scan_complex_column(scan_st, "alipay_trade_num/2", "alipay_trade_num/2", 1);
      //ob_scan_complex_column(scan_st, "alipay_auction_num/2", "alipay_auction_num/2", 1);
      //ob_scan_complex_column(scan_st, "alipay_trade_amt/2", "alipay_trade_amt/2", 1);
      //ob_scan_complex_column(scan_st, "alipay_winner_num/2", "alipay_winner_num/2", 1);
      //ob_scan_complex_column(scan_st, "today_trade_num/2", "today_trade_num/2", 1);
      //ob_scan_complex_column(scan_st, "today_trade_amt/2", "today_trade_amt/2", 1);
      //ob_scan_complex_column(scan_st, "today_auction_num/2", "today_auction_num/2", 1);
      //ob_scan_complex_column(scan_st, "plot_gmv_trade_num/2", "plot_gmv_trade_num/2", 1);
      //ob_scan_complex_column(scan_st, "plot_gmv_trade_amt/2", "plot_gmv_trade_amt/2", 1);
      //ob_scan_complex_column(scan_st, "plot_gmv_auction_num/2", "plot_gmv_auction_num/2", 1);
      //ob_scan_complex_column(scan_st, "plot_gmv_winner_num/2", "plot_gmv_winner_num/2", 1);
      //ob_scan_complex_column(scan_st, "plot_alipay_trade_num/2", "plot_alipay_trade_num/2", 1);
      //ob_scan_complex_column(scan_st, "plot_alipay_trade_amt/2", "plot_alipay_trade_amt/2", 1);
      //ob_scan_complex_column(scan_st, "plot_alipay_auction_num/2", "plot_alipay_auction_num/2", 1);
      //ob_scan_complex_column(scan_st, "plot_alipay_winner_num/2", "plot_alipay_winner_num/2", 1);
      //ob_scan_complex_column(scan_st, "noplot_gmv_trade_num/2", "noplot_gmv_trade_num/2", 1);
      //ob_scan_complex_column(scan_st, "noplot_gmv_trade_amt/2", "noplot_gmv_trade_amt/2", 1);
      //ob_scan_complex_column(scan_st, "noplot_gmv_auction_num/2", "noplot_gmv_auction_num/2", 1);
      //ob_scan_complex_column(scan_st, "noplot_gmv_winner_num/2", "noplot_gmv_winner_num/2", 1);
      //ob_scan_complex_column(scan_st, "noplot_alipay_trade_num/2", "noplot_alipay_trade_num/2", 1);
      //ob_scan_complex_column(scan_st, "noplot_alipay_trade_amt/2", "noplot_alipay_trade_amt/2", 1);
      //ob_scan_complex_column(scan_st, "noplot_alipay_auction_num/2", "noplot_alipay_auction_num/2", 1);
      //ob_scan_complex_column(scan_st, "noplot_alipay_winner_num/2", "noplot_alipay_winner_num/2", 1);
      //ob_scan_complex_column(scan_st, "plot_type/2", "plot_type/2", 1);

      //ob_scan_set_where(scan_st, "day>=100");
      //ob_scan_set_where(scan_st, "unit_id<100");
      //ob_scan_orderby_column(scan_st, "COUNTgmv_trade_num", OB_DESC);
      //ob_scan_orderby_column(scan_st, "today_trade_num", OB_DESC);
      //ob_scan_orderby_column(scan_st, "SUMalipay_auction_num", OB_DESC);
      //ob_scan_set_limit(scan_st, 0, 25);

      OB_GROUPBY_PARAM* groupby = ob_get_ob_groupby_param(scan_st);
      if (NULL == groupby)
      {
        fprintf(stderr, "groupby is NULL error\n");
      }
      else
      {
        ob_groupby_column(groupby, "unit_id", 1);
        //ob_groupby_column(groupby, "shopid", 1);
        //ob_groupby_column(groupby, "promotion", 1);
        ob_groupby_column(groupby, "aucid", 1);
        //ob_aggregate_column(groupby, OB_SUM, "ipv", "SUMipv", 1);
        //ob_aggregate_column(groupby, OB_SUM, "iuv", "SUMiuv", 1);
        //ob_aggregate_column(groupby, OB_SUM, "coll_num", "SUMcoll_num", 1);
        //ob_aggregate_column(groupby, OB_SUM, "today_trade_num", "SUMtoday_trade_num" , 1);
        //ob_aggregate_column(groupby, OB_SUM, "today_trade_amt", "SUMtoday_trade_amt", 1);
        //ob_aggregate_column(groupby, OB_SUM, "today_auction_num", "SUMtoday_auction_num", 1);
        //ob_aggregate_column(groupby, OB_SUM, "day", "SUMday", 1);
        //ob_aggregate_column(groupby, OB_SUM, "aucid", "SUMaucid", 1);
        //ob_aggregate_column(groupby, OB_SUM, "promotion", "SUMpromotion", 1);
        //ob_aggregate_column(groupby, OB_SUM, "unit_id", "SUMunit_id", 1);
        //ob_aggregate_column(groupby, OB_SUM, "shopid", "SUMshopid", 1);
        //ob_aggregate_column(groupby, OB_SUM, "ipv", "SUMipv", 1);
        //ob_aggregate_column(groupby, OB_SUM, "iuv", "SUMiuv", 1);
        //ob_aggregate_column(groupby, OB_SUM, "coll_num", "SUMcoll_num", 1);
        //ob_aggregate_column(groupby, OB_SUM, "gmv_trade_num", "SUMgmv_trade_num", 1);
        //ob_aggregate_column(groupby, OB_SUM, "gmv_auction_num", "SUMgmv_auction_num", 1);
        //ob_aggregate_column(groupby, OB_SUM, "gmv_trade_amt", "SUMgmv_trade_amt", 1);
        //ob_aggregate_column(groupby, OB_SUM, "gmv_winner_num", "SUMgmv_winner_num", 1);
        //ob_aggregate_column(groupby, OB_SUM, "alipay_trade_num", "SUMalipay_trade_num", 1);
        //ob_aggregate_column(groupby, OB_SUM, "alipay_auction_num", "SUMalipay_auction_num", 1);
        //ob_aggregate_column(groupby, OB_SUM, "alipay_trade_amt", "SUMalipay_trade_amt", 1);
        //ob_aggregate_column(groupby, OB_SUM, "alipay_winner_num", "SUMalipay_winner_num", 1);
        //ob_aggregate_column(groupby, OB_SUM, "today_trade_num", "SUMtoday_trade_num", 1);
        //ob_aggregate_column(groupby, OB_SUM, "today_trade_amt", "SUMtoday_trade_amt", 1);
        //ob_aggregate_column(groupby, OB_SUM, "today_auction_num", "SUMtoday_auction_num", 1);
        //ob_aggregate_column(groupby, OB_SUM, "plot_gmv_trade_num", "SUMplot_gmv_trade_num", 1);
        //ob_aggregate_column(groupby, OB_SUM, "plot_gmv_trade_amt", "SUMplot_gmv_trade_amt", 1);
        //ob_aggregate_column(groupby, OB_SUM, "plot_gmv_auction_num", "SUMplot_gmv_auction_num", 1);
        //ob_aggregate_column(groupby, OB_SUM, "plot_gmv_winner_num", "SUMplot_gmv_winner_num", 1);
        //ob_aggregate_column(groupby, OB_SUM, "plot_alipay_trade_num", "SUMplot_alipay_trade_num", 1);
        //ob_aggregate_column(groupby, OB_SUM, "plot_alipay_trade_amt", "SUMplot_alipay_trade_amt", 1);
        //ob_aggregate_column(groupby, OB_SUM, "plot_alipay_auction_num", "SUMplot_alipay_auction_num", 1);
        //ob_aggregate_column(groupby, OB_SUM, "plot_alipay_winner_num", "SUMplot_alipay_winner_num", 1);
        //ob_aggregate_column(groupby, OB_SUM, "noplot_gmv_trade_num", "SUMnoplot_gmv_trade_num", 1);
        //ob_aggregate_column(groupby, OB_SUM, "noplot_gmv_trade_amt", "SUMnoplot_gmv_trade_amt", 1);
        //ob_aggregate_column(groupby, OB_SUM, "noplot_gmv_auction_num", "SUMnoplot_gmv_auction_num", 1);
        //ob_aggregate_column(groupby, OB_SUM, "noplot_gmv_winner_num", "SUMnoplot_gmv_winner_num", 1);
        //ob_aggregate_column(groupby, OB_SUM, "noplot_alipay_trade_num", "SUMnoplot_alipay_trade_num", 1);
        //ob_aggregate_column(groupby, OB_SUM, "noplot_alipay_trade_amt", "SUMnoplot_alipay_trade_amt", 1);
        //ob_aggregate_column(groupby, OB_SUM, "noplot_alipay_auction_num", "SUMnoplot_alipay_auction_num", 1);
        //ob_aggregate_column(groupby, OB_SUM, "noplot_alipay_winner_num", "SUMnoplot_alipay_winner_num", 1);
        //ob_aggregate_column(groupby, OB_SUM, "plot_type", "SUMplot_type", 1);


        //ob_scan_orderby_column(scan_st, "SUMtoday_trade_amt", OB_DESC);
        //ob_scan_orderby_column(scan_st, "unit_id", OB_DESC);
        if (OB_ERR_SUCCESS != ob_errno())
        {
          fprintf(stderr, "ob_scan error: %s\n", ob_error());
        }
        else
        {
          int64_t start_time = microseconds();
          OB_RES* res_st = ob_exec_scan(ob, scan_st);
          int64_t end_time = microseconds();
          if (NULL == res_st)
          {
            fprintf(stderr, "ob_exec_scan error(%d): %s\n", ob_errno(), ob_error());
          }
          else
          {
            pres(res_st);
            printf("scan succ: row_num=%ld elapsed_time=%dus\n",
                ob_fetch_row_num(res_st), end_time - start_time);
            ob_release_res_st(ob, res_st);
          }
        }
      }
      ob_release_scan_st(ob, scan_st);
    }
  }

  ob_api_destroy(ob);

  return 0;
}
