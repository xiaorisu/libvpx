/*
 * Copyright (c) 2016, Alliance for Open Media. All rights reserved
 *
 * This source code is subject to the terms of the BSD 2 Clause License and
 * the Alliance for Open Media Patent License 1.0. If the BSD 2 Clause License
 * was not distributed with this source code in the LICENSE file, you can
 * obtain it at www.aomedia.org/license/software. If the Alliance for Open
 * Media Patent License 1.0 was not distributed with this source code in the
 * PATENTS file, you can obtain it at www.aomedia.org/license/patent.
 */
#include <assert.h>

#include "av1/encoder/cost.h"
#if CONFIG_ANS
#include "av1/common/ans.h"
#endif  // CONFIG_ANS
#include "av1/common/entropy.h"

/* round(-log2(i/256.) * (1 << AV1_PROB_COST_SHIFT))
   Begins with a bogus entry for simpler addressing. */
const uint16_t av1_prob_cost[256] = {
  4096, 4096, 3584, 3284, 3072, 2907, 2772, 2659, 2560, 2473, 2395, 2325, 2260,
  2201, 2147, 2096, 2048, 2003, 1961, 1921, 1883, 1847, 1813, 1780, 1748, 1718,
  1689, 1661, 1635, 1609, 1584, 1559, 1536, 1513, 1491, 1470, 1449, 1429, 1409,
  1390, 1371, 1353, 1335, 1318, 1301, 1284, 1268, 1252, 1236, 1221, 1206, 1192,
  1177, 1163, 1149, 1136, 1123, 1110, 1097, 1084, 1072, 1059, 1047, 1036, 1024,
  1013, 1001, 990,  979,  968,  958,  947,  937,  927,  917,  907,  897,  887,
  878,  868,  859,  850,  841,  832,  823,  814,  806,  797,  789,  780,  772,
  764,  756,  748,  740,  732,  724,  717,  709,  702,  694,  687,  680,  673,
  665,  658,  651,  644,  637,  631,  624,  617,  611,  604,  598,  591,  585,
  578,  572,  566,  560,  554,  547,  541,  535,  530,  524,  518,  512,  506,
  501,  495,  489,  484,  478,  473,  467,  462,  456,  451,  446,  441,  435,
  430,  425,  420,  415,  410,  405,  400,  395,  390,  385,  380,  375,  371,
  366,  361,  356,  352,  347,  343,  338,  333,  329,  324,  320,  316,  311,
  307,  302,  298,  294,  289,  285,  281,  277,  273,  268,  264,  260,  256,
  252,  248,  244,  240,  236,  232,  228,  224,  220,  216,  212,  209,  205,
  201,  197,  194,  190,  186,  182,  179,  175,  171,  168,  164,  161,  157,
  153,  150,  146,  143,  139,  136,  132,  129,  125,  122,  119,  115,  112,
  109,  105,  102,  99,   95,   92,   89,   86,   82,   79,   76,   73,   70,
  66,   63,   60,   57,   54,   51,   48,   45,   42,   38,   35,   32,   29,
  26,   23,   20,   18,   15,   12,   9,    6,    3
};

#if CONFIG_ANS
// round(-log2(i/1024.) * (1 << AV1_PROB_COST_SHIFT))
static const uint16_t av1_prob_cost10[1024] = {
  5120, 5120, 4608, 4308, 4096, 3931, 3796, 3683, 3584, 3497, 3419, 3349, 3284,
  3225, 3171, 3120, 3072, 3027, 2985, 2945, 2907, 2871, 2837, 2804, 2772, 2742,
  2713, 2685, 2659, 2633, 2608, 2583, 2560, 2537, 2515, 2494, 2473, 2453, 2433,
  2414, 2395, 2377, 2359, 2342, 2325, 2308, 2292, 2276, 2260, 2245, 2230, 2216,
  2201, 2187, 2173, 2160, 2147, 2134, 2121, 2108, 2096, 2083, 2071, 2060, 2048,
  2037, 2025, 2014, 2003, 1992, 1982, 1971, 1961, 1951, 1941, 1931, 1921, 1911,
  1902, 1892, 1883, 1874, 1865, 1856, 1847, 1838, 1830, 1821, 1813, 1804, 1796,
  1788, 1780, 1772, 1764, 1756, 1748, 1741, 1733, 1726, 1718, 1711, 1704, 1697,
  1689, 1682, 1675, 1668, 1661, 1655, 1648, 1641, 1635, 1628, 1622, 1615, 1609,
  1602, 1596, 1590, 1584, 1578, 1571, 1565, 1559, 1554, 1548, 1542, 1536, 1530,
  1525, 1519, 1513, 1508, 1502, 1497, 1491, 1486, 1480, 1475, 1470, 1465, 1459,
  1454, 1449, 1444, 1439, 1434, 1429, 1424, 1419, 1414, 1409, 1404, 1399, 1395,
  1390, 1385, 1380, 1376, 1371, 1367, 1362, 1357, 1353, 1348, 1344, 1340, 1335,
  1331, 1326, 1322, 1318, 1313, 1309, 1305, 1301, 1297, 1292, 1288, 1284, 1280,
  1276, 1272, 1268, 1264, 1260, 1256, 1252, 1248, 1244, 1240, 1236, 1233, 1229,
  1225, 1221, 1218, 1214, 1210, 1206, 1203, 1199, 1195, 1192, 1188, 1185, 1181,
  1177, 1174, 1170, 1167, 1163, 1160, 1156, 1153, 1149, 1146, 1143, 1139, 1136,
  1133, 1129, 1126, 1123, 1119, 1116, 1113, 1110, 1106, 1103, 1100, 1097, 1094,
  1090, 1087, 1084, 1081, 1078, 1075, 1072, 1069, 1066, 1062, 1059, 1056, 1053,
  1050, 1047, 1044, 1042, 1039, 1036, 1033, 1030, 1027, 1024, 1021, 1018, 1015,
  1013, 1010, 1007, 1004, 1001, 998,  996,  993,  990,  987,  985,  982,  979,
  977,  974,  971,  968,  966,  963,  960,  958,  955,  953,  950,  947,  945,
  942,  940,  937,  934,  932,  929,  927,  924,  922,  919,  917,  914,  912,
  909,  907,  904,  902,  899,  897,  895,  892,  890,  887,  885,  883,  880,
  878,  876,  873,  871,  868,  866,  864,  861,  859,  857,  855,  852,  850,
  848,  845,  843,  841,  839,  836,  834,  832,  830,  828,  825,  823,  821,
  819,  817,  814,  812,  810,  808,  806,  804,  801,  799,  797,  795,  793,
  791,  789,  787,  785,  783,  780,  778,  776,  774,  772,  770,  768,  766,
  764,  762,  760,  758,  756,  754,  752,  750,  748,  746,  744,  742,  740,
  738,  736,  734,  732,  730,  728,  726,  724,  723,  721,  719,  717,  715,
  713,  711,  709,  707,  706,  704,  702,  700,  698,  696,  694,  693,  691,
  689,  687,  685,  683,  682,  680,  678,  676,  674,  673,  671,  669,  667,
  665,  664,  662,  660,  658,  657,  655,  653,  651,  650,  648,  646,  644,
  643,  641,  639,  637,  636,  634,  632,  631,  629,  627,  626,  624,  622,
  621,  619,  617,  616,  614,  612,  611,  609,  607,  606,  604,  602,  601,
  599,  598,  596,  594,  593,  591,  590,  588,  586,  585,  583,  582,  580,
  578,  577,  575,  574,  572,  571,  569,  567,  566,  564,  563,  561,  560,
  558,  557,  555,  554,  552,  550,  549,  547,  546,  544,  543,  541,  540,
  538,  537,  535,  534,  532,  531,  530,  528,  527,  525,  524,  522,  521,
  519,  518,  516,  515,  513,  512,  511,  509,  508,  506,  505,  503,  502,
  501,  499,  498,  496,  495,  493,  492,  491,  489,  488,  486,  485,  484,
  482,  481,  480,  478,  477,  475,  474,  473,  471,  470,  469,  467,  466,
  465,  463,  462,  460,  459,  458,  456,  455,  454,  452,  451,  450,  448,
  447,  446,  444,  443,  442,  441,  439,  438,  437,  435,  434,  433,  431,
  430,  429,  428,  426,  425,  424,  422,  421,  420,  419,  417,  416,  415,
  414,  412,  411,  410,  409,  407,  406,  405,  404,  402,  401,  400,  399,
  397,  396,  395,  394,  392,  391,  390,  389,  387,  386,  385,  384,  383,
  381,  380,  379,  378,  377,  375,  374,  373,  372,  371,  369,  368,  367,
  366,  365,  364,  362,  361,  360,  359,  358,  356,  355,  354,  353,  352,
  351,  349,  348,  347,  346,  345,  344,  343,  341,  340,  339,  338,  337,
  336,  335,  333,  332,  331,  330,  329,  328,  327,  326,  324,  323,  322,
  321,  320,  319,  318,  317,  316,  314,  313,  312,  311,  310,  309,  308,
  307,  306,  305,  303,  302,  301,  300,  299,  298,  297,  296,  295,  294,
  293,  292,  291,  289,  288,  287,  286,  285,  284,  283,  282,  281,  280,
  279,  278,  277,  276,  275,  274,  273,  272,  271,  269,  268,  267,  266,
  265,  264,  263,  262,  261,  260,  259,  258,  257,  256,  255,  254,  253,
  252,  251,  250,  249,  248,  247,  246,  245,  244,  243,  242,  241,  240,
  239,  238,  237,  236,  235,  234,  233,  232,  231,  230,  229,  228,  227,
  226,  225,  224,  223,  222,  221,  220,  219,  218,  217,  216,  215,  214,
  213,  212,  212,  211,  210,  209,  208,  207,  206,  205,  204,  203,  202,
  201,  200,  199,  198,  197,  196,  195,  194,  194,  193,  192,  191,  190,
  189,  188,  187,  186,  185,  184,  183,  182,  181,  181,  180,  179,  178,
  177,  176,  175,  174,  173,  172,  171,  170,  170,  169,  168,  167,  166,
  165,  164,  163,  162,  161,  161,  160,  159,  158,  157,  156,  155,  154,
  153,  152,  152,  151,  150,  149,  148,  147,  146,  145,  145,  144,  143,
  142,  141,  140,  139,  138,  138,  137,  136,  135,  134,  133,  132,  132,
  131,  130,  129,  128,  127,  126,  125,  125,  124,  123,  122,  121,  120,
  120,  119,  118,  117,  116,  115,  114,  114,  113,  112,  111,  110,  109,
  109,  108,  107,  106,  105,  104,  104,  103,  102,  101,  100,  99,   99,
  98,   97,   96,   95,   95,   94,   93,   92,   91,   90,   90,   89,   88,
  87,   86,   86,   85,   84,   83,   82,   82,   81,   80,   79,   78,   78,
  77,   76,   75,   74,   74,   73,   72,   71,   70,   70,   69,   68,   67,
  66,   66,   65,   64,   63,   62,   62,   61,   60,   59,   59,   58,   57,
  56,   55,   55,   54,   53,   52,   52,   51,   50,   49,   48,   48,   47,
  46,   45,   45,   44,   43,   42,   42,   41,   40,   39,   38,   38,   37,
  36,   35,   35,   34,   33,   32,   32,   31,   30,   29,   29,   28,   27,
  26,   26,   25,   24,   23,   23,   22,   21,   20,   20,   19,   18,   18,
  17,   16,   15,   15,   14,   13,   12,   12,   11,   10,   9,    9,    8,
  7,    7,    6,    5,    4,    4,    3,    2,    1,    1
};
#endif  // CONFIG_ANS

static void cost(int *costs, aom_tree tree, const aom_prob *probs, int i,
                 int c) {
  const aom_prob prob = probs[i / 2];
  int b;

  assert(prob != 0);
  for (b = 0; b <= 1; ++b) {
    const int cc = c + av1_cost_bit(prob, b);
    const aom_tree_index ii = tree[i + b];

    if (ii <= 0)
      costs[-ii] = cc;
    else
      cost(costs, tree, probs, ii, cc);
  }
}

#if CONFIG_ANS
void av1_cost_tokens_ans(int *costs, const aom_prob *tree_probs,
                         const rans_dec_lut token_cdf, int skip_eob) {
  int c_tree = 0;  // Cost of the "tree" nodes EOB and ZERO.
  int i;
  costs[EOB_TOKEN] = av1_cost_bit(tree_probs[0], 0);
  if (!skip_eob) c_tree = av1_cost_bit(tree_probs[0], 1);
  for (i = ZERO_TOKEN; i <= CATEGORY6_TOKEN; ++i) {
    const int p = token_cdf[i + 1] - token_cdf[i];
    costs[i] = c_tree + av1_prob_cost10[p];
  }
}
#endif  // CONFIG_ANS

void av1_cost_tokens(int *costs, const aom_prob *probs, aom_tree tree) {
  cost(costs, tree, probs, 0, 0);
}

void av1_cost_tokens_skip(int *costs, const aom_prob *probs, aom_tree tree) {
  assert(tree[0] <= 0 && tree[1] > 0);

  costs[-tree[0]] = av1_cost_bit(probs[0], 0);
  cost(costs, tree, probs, 2, 0);
}
