/*
 *  Copyright (c) 2015 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "av1/common/divide.h"

/* Constants for divide by multiply for small divisors generated with:
void init_fastdiv() {
  int i;
  for (i = 3; i < 256; ++i) {
    const int s = 31 ^ __builtin_clz(2 * i + 1);
    const unsigned long long base = (1ull << (sizeof(unsigned) * 8 + s)) - 1;
    fastdiv_tab[i].mult = (base / i + 1) & 0xFFFFFFFF;
    fastdiv_tab[i].shift = s;
  }
  for (i = 0; i < 8; ++i) {
    fastdiv_tab[1 << i].mult = 0;
    fastdiv_tab[1 << i].shift = i;
  }
}
*/
const struct fastdiv_elem av1_fastdiv_tab[256] = {
  { 0, 0 },           { 0, 0 },           { 0, 1 },
  { 1431655766, 2 },  { 0, 2 },           { 2576980378u, 3 },
  { 1431655766, 3 },  { 613566757, 3 },   { 0, 3 },
  { 3340530120u, 4 }, { 2576980378u, 4 }, { 1952257862, 4 },
  { 1431655766, 4 },  { 991146300, 4 },   { 613566757, 4 },
  { 286331154u, 4 },  { 0, 4 },           { 3789677026u, 5 },
  { 3340530120u, 5 }, { 2938661835u, 5 }, { 2576980378u, 5 },
  { 2249744775u, 5 }, { 1952257862, 5 },  { 1680639377, 5 },
  { 1431655766, 5 },  { 1202590843, 5 },  { 991146300, 5 },
  { 795364315, 5 },   { 613566757, 5 },   { 444306962, 5 },
  { 286331154, 5 },   { 138547333, 5 },   { 0, 5 },
  { 4034666248u, 6 }, { 3789677026u, 6 }, { 3558687189u, 6 },
  { 3340530120u, 6 }, { 3134165325u, 6 }, { 2938661835u, 6 },
  { 2753184165u, 6 }, { 2576980378u, 6 }, { 2409371898u, 6 },
  { 2249744775u, 6 }, { 2097542168u, 6 }, { 1952257862, 6 },
  { 1813430637, 6 },  { 1680639377, 6 },  { 1553498810, 6 },
  { 1431655766, 6 },  { 1314785907, 6 },  { 1202590843, 6 },
  { 1094795586, 6 },  { 991146300, 6 },   { 891408307, 6 },
  { 795364315, 6 },   { 702812831, 6 },   { 613566757, 6 },
  { 527452125, 6 },   { 444306962, 6 },   { 363980280, 6 },
  { 286331154, 6 },   { 211227900, 6 },   { 138547333, 6 },
  { 68174085, 6 },    { 0, 6 },           { 4162814457u, 7 },
  { 4034666248u, 7 }, { 3910343360u, 7 }, { 3789677026u, 7 },
  { 3672508268u, 7 }, { 3558687189u, 7 }, { 3448072337u, 7 },
  { 3340530120u, 7 }, { 3235934265u, 7 }, { 3134165325u, 7 },
  { 3035110223u, 7 }, { 2938661835u, 7 }, { 2844718599u, 7 },
  { 2753184165u, 7 }, { 2663967058u, 7 }, { 2576980378u, 7 },
  { 2492141518u, 7 }, { 2409371898u, 7 }, { 2328596727u, 7 },
  { 2249744775u, 7 }, { 2172748162u, 7 }, { 2097542168, 7 },
  { 2024065048, 7 },  { 1952257862, 7 },  { 1882064321, 7 },
  { 1813430637, 7 },  { 1746305385, 7 },  { 1680639377, 7 },
  { 1616385542, 7 },  { 1553498810, 7 },  { 1491936009, 7 },
  { 1431655766, 7 },  { 1372618415, 7 },  { 1314785907, 7 },
  { 1258121734, 7 },  { 1202590843, 7 },  { 1148159575, 7 },
  { 1094795586, 7 },  { 1042467791, 7 },  { 991146300, 7 },
  { 940802361, 7 },   { 891408307, 7 },   { 842937507, 7 },
  { 795364315, 7 },   { 748664025, 7 },   { 702812831, 7 },
  { 657787785, 7 },   { 613566757, 7 },   { 570128403, 7 },
  { 527452125, 7 },   { 485518043, 7 },   { 444306962, 7 },
  { 403800345, 7 },   { 363980280, 7 },   { 324829460, 7 },
  { 286331154, 7 },   { 248469183, 7 },   { 211227900, 7 },
  { 174592167, 7 },   { 138547333, 7 },   { 103079216, 7 },
  { 68174085, 7 },    { 33818641, 7 },    { 0, 7 },
  { 4228378656u, 8 }, { 4162814457u, 8 }, { 4098251237u, 8 },
  { 4034666248u, 8 }, { 3972037425u, 8 }, { 3910343360u, 8 },
  { 3849563281u, 8 }, { 3789677026u, 8 }, { 3730665024u, 8 },
  { 3672508268u, 8 }, { 3615188300u, 8 }, { 3558687189u, 8 },
  { 3502987511u, 8 }, { 3448072337u, 8 }, { 3393925206u, 8 },
  { 3340530120u, 8 }, { 3287871517u, 8 }, { 3235934265u, 8 },
  { 3184703642u, 8 }, { 3134165325u, 8 }, { 3084305374u, 8 },
  { 3035110223u, 8 }, { 2986566663u, 8 }, { 2938661835u, 8 },
  { 2891383213u, 8 }, { 2844718599u, 8 }, { 2798656110u, 8 },
  { 2753184165u, 8 }, { 2708291480u, 8 }, { 2663967058u, 8 },
  { 2620200175u, 8 }, { 2576980378u, 8 }, { 2534297473u, 8 },
  { 2492141518u, 8 }, { 2450502814u, 8 }, { 2409371898u, 8 },
  { 2368739540u, 8 }, { 2328596727u, 8 }, { 2288934667u, 8 },
  { 2249744775u, 8 }, { 2211018668u, 8 }, { 2172748162u, 8 },
  { 2134925265u, 8 }, { 2097542168, 8 },  { 2060591247, 8 },
  { 2024065048, 8 },  { 1987956292, 8 },  { 1952257862, 8 },
  { 1916962805, 8 },  { 1882064321, 8 },  { 1847555765, 8 },
  { 1813430637, 8 },  { 1779682582, 8 },  { 1746305385, 8 },
  { 1713292966, 8 },  { 1680639377, 8 },  { 1648338801, 8 },
  { 1616385542, 8 },  { 1584774030, 8 },  { 1553498810, 8 },
  { 1522554545, 8 },  { 1491936009, 8 },  { 1461638086, 8 },
  { 1431655766, 8 },  { 1401984144, 8 },  { 1372618415, 8 },
  { 1343553873, 8 },  { 1314785907, 8 },  { 1286310003, 8 },
  { 1258121734, 8 },  { 1230216764, 8 },  { 1202590843, 8 },
  { 1175239808, 8 },  { 1148159575, 8 },  { 1121346142, 8 },
  { 1094795586, 8 },  { 1068504060, 8 },  { 1042467791, 8 },
  { 1016683080, 8 },  { 991146300, 8 },   { 965853890, 8 },
  { 940802361, 8 },   { 915988286, 8 },   { 891408307, 8 },
  { 867059126, 8 },   { 842937507, 8 },   { 819040276, 8 },
  { 795364315, 8 },   { 771906565, 8 },   { 748664025, 8 },
  { 725633745, 8 },   { 702812831, 8 },   { 680198441, 8 },
  { 657787785, 8 },   { 635578121, 8 },   { 613566757, 8 },
  { 591751050, 8 },   { 570128403, 8 },   { 548696263, 8 },
  { 527452125, 8 },   { 506393524, 8 },   { 485518043, 8 },
  { 464823301, 8 },   { 444306962, 8 },   { 423966729, 8 },
  { 403800345, 8 },   { 383805589, 8 },   { 363980280, 8 },
  { 344322273, 8 },   { 324829460, 8 },   { 305499766, 8 },
  { 286331154, 8 },   { 267321616, 8 },   { 248469183, 8 },
  { 229771913, 8 },   { 211227900, 8 },   { 192835267, 8 },
  { 174592167, 8 },   { 156496785, 8 },   { 138547333, 8 },
  { 120742053, 8 },   { 103079216, 8 },   { 85557118, 8 },
  { 68174085, 8 },    { 50928466, 8 },    { 33818641, 8 },
  { 16843010, 8 },
};
