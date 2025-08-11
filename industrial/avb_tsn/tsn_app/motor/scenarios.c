/*
 * Copyright 2019, 2024-2025 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "scenarios.h"
#include "tsn_tasks_config.h"
#include "log.h"

#define APP_PERIOD_250_US 250000

// --------------BLOCKS-----------------

struct scenario_block blocks_0[] = {
    [0] = {
        .pos_target = -0.125,
        .after_delay = 1000,
        .speed_max = 500.0,
    },
    [1] = {
        .pos_target = -0.750,
        .after_delay = 1000,
        .speed_max = 500.0,
    },
    [2] = {
        .pos_target = -0.250,
        .after_delay = 1000,
        .speed_max = 500.0,
    },
    [3] = {
        .pos_target = 1.875,
        .after_delay = 1000,
        .speed_max = 200.0,
    },
    [4] = {
        .pos_target = -0.625,
        .after_delay = 1000,
        .speed_max = 200.0,
    },
    [5] = {
        .pos_target = -0.375,
        .after_delay = 1000,
        .speed_max = 500.0,
    },
    [6] = {
        .pos_target = -0.250,
        .after_delay = 1000,
        .speed_max = 500.0,
    },
    [7] = {
        .pos_target = 20.5,
        .after_delay = 1000,
        .speed_max = 500.0,
    },
    [8] = {
        .pos_target = 7.25,
        .after_delay = 1000,
        .speed_max = 500.0,
    },
    [9] = {
        .pos_target = 1.75,
        .after_delay = 1000,
        .speed_max = 500.0,
    },
    [10] = {
        .pos_target = 1.625,
        .after_delay = 1000,
        .speed_max = 5.0,
    },
    [11] = {
        .pos_target = 0.0,
        .after_delay = 1000,
        .speed_max = 500.0,
    },
    [12] = {
        .pos_target = 0.25,
        .after_delay = 1000,
        .speed_max = 10.0,
    },
    [13] = {
        .pos_target = -0.25,
        .after_delay = 1000,
        .speed_max = 10.0,
    },
    [14] = {
        .pos_target = -0.125,
        .after_delay = 1000,
        .speed_max = 500.0,
    },
    [15] = {
        .pos_target = 0.125,
        .after_delay = 1000,
        .speed_max = 500.0,
    },
    [16] = {
        .pos_target = 0.250,
        .after_delay = 1000,
        .speed_max = 500.0,
    },
    [17] = {
        .pos_target = 0.250,
        .after_delay = 1000,
        .speed_max = 500.0,
    },
    [18] = {
        .pos_target = 0.0,
        .after_delay = 1000,
        .speed_max = 500.0,
    },
};

struct scenario_block blocks_0_rev[] = {
    [0] = {
        .pos_target = 0.125,
        .after_delay = 1000,
        .speed_max = 500.0,
    },
    [1] = {
        .pos_target = 0.750,
        .after_delay = 1000,
        .speed_max = 500.0,
    },
    [2] = {
        .pos_target = 0.250,
        .after_delay = 1000,
        .speed_max = 500.0,
    },
    [3] = {
        .pos_target = -1.875,
        .after_delay = 1000,
        .speed_max = 200.0,
    },
    [4] = {
        .pos_target = 0.625,
        .after_delay = 1000,
        .speed_max = 200.0,
    },
    [5] = {
        .pos_target = 0.375,
        .after_delay = 1000,
        .speed_max = 500.0,
    },
    [6] = {
        .pos_target = 0.250,
        .after_delay = 1000,
        .speed_max = 500.0,
    },
    [7] = {
        .pos_target = -20.5,
        .after_delay = 1000,
        .speed_max = 500.0,
    },
    [8] = {
        .pos_target = -7.25,
        .after_delay = 1000,
        .speed_max = 500.0,
    },
    [9] = {
        .pos_target = -1.75,
        .after_delay = 1000,
        .speed_max = 500.0,
    },
    [10] = {
        .pos_target = -1.625,
        .after_delay = 1000,
        .speed_max = 5.0,
    },
    [11] = {
        .pos_target = 0.0,
        .after_delay = 1000,
        .speed_max = 500.0,
    },
    [12] = {
        .pos_target = 0.25,
        .after_delay = 1000,
        .speed_max = 10.0,
    },
    [13] = {
        .pos_target = -0.25,
        .after_delay = 1000,
        .speed_max = 10.0,
    },
    [14] = {
        .pos_target = -0.125,
        .after_delay = 1000,
        .speed_max = 500.0,
    },
    [15] = {
        .pos_target = 0.125,
        .after_delay = 1000,
        .speed_max = 500.0,
    },
    [16] = {
        .pos_target = 0.250,
        .after_delay = 1000,
        .speed_max = 500.0,
    },
    [17] = {
        .pos_target = 0.250,
        .after_delay = 1000,
        .speed_max = 500.0,
    },
    [18] = {
        .pos_target = 0.0,
        .after_delay = 1000,
        .speed_max = 500.0,
    },
};

struct scenario_block blocks_1[] = {
    [0] = {
        .pos_target = 0.250,
        .after_delay = 1000,
        .speed_max = 2000.0,
    },
    [1] = {
        .pos_target = 0.5,
        .after_delay = 1000,
        .speed_max = 2000.0,
    },
    [2] = {
        .pos_target = 0.75,
        .after_delay = 1000,
        .speed_max = 2000.0,
    },
    [3] = {
        .pos_target = 1.0,
        .after_delay = 1000,
        .speed_max = 2000.0,
    },
    [4] = {
        .pos_target = 1.25,
        .after_delay = 1000,
        .speed_max = 2000.0,
    },
    [5] = {
        .pos_target = 1.5,
        .after_delay = 1000,
        .speed_max = 2000.0,
    },
    [6] = {
        .pos_target = 1.75,
        .after_delay = 1000,
        .speed_max = 2000.0,
    },
    [7] = {
        .pos_target = 2.0,
        .after_delay = 1000,
        .speed_max = 2000.0,
    },
    [8] = {
        .pos_target = 2.25,
        .after_delay = 1000,
        .speed_max = 2000.0,
    },
    [9] = {
        .pos_target = 2.5,
        .after_delay = 1000,
        .speed_max = 2000.0,
    },
    [10] = {
        .pos_target = 2.75,
        .after_delay = 1000,
        .speed_max = 2000.0,
    },
    [11] = {
        .pos_target = 3.0,
        .after_delay = 1000,
        .speed_max = 2000.0,
    },
    [12] = {
        .pos_target = 3.25,
        .after_delay = 1000,
        .speed_max = 2000.0,
    },
    [13] = {
        .pos_target = 3.5,
        .after_delay = 1000,
        .speed_max = 2000.0,
    },
    [14] = {
        .pos_target = 3.75,
        .after_delay = 1000,
        .speed_max = 2000.0,
    },
    [15] = {
        .pos_target = 4.0,
        .after_delay = 1000,
        .speed_max = 2000.0,
    },
    [16] = {
        .pos_target = 4.25,
        .after_delay = 1000,
        .speed_max = 2000.0,
    },
    [17] = {
        .pos_target = 4.5,
        .after_delay = 1000,
        .speed_max = 2000.0,
    },
    [18] = {
        .pos_target = 0.0,
        .after_delay = 1000,
        .speed_max = 2000.0,
    },
};

struct scenario_block blocks_2[] = {
    [0] = {
        .pos_target = 0.125,
        .after_delay = 0,
        .speed_max = 8.0,
    },
    [1] = {
        .pos_target = 0.250,
        .after_delay = 0,
        .speed_max = 8.0,
    },
    [2] = {
        .pos_target = 0.375,
        .after_delay = 0,
        .speed_max = 8.0,
    },
    [3] = {
        .pos_target = 0.500,
        .after_delay = 0,
        .speed_max = 16.0,
    },
    [4] = {
        .pos_target = 0.625,
        .after_delay = 0,
        .speed_max = 16.0,
    },
    [5] = {
        .pos_target = 0.750,
        .after_delay = 0,
        .speed_max = 16.0,
    },
    [6] = {
        .pos_target = 0.875,
        .after_delay = 0,
        .speed_max = 32.0,
    },
    [7] = {
        .pos_target = 1.0,
        .after_delay = 0,
        .speed_max = 32.0,
    },
    [8] = {
        .pos_target = 1.125,
        .after_delay = 0,
        .speed_max = 32.0,
    },
    [9] = {
        .pos_target = 1.250,
        .after_delay = 0,
        .speed_max = 64.0,
    },
    [10] = {
        .pos_target = 1.375,
        .after_delay = 0,
        .speed_max = 64.0,
    },
    [11] = {
        .pos_target = 1.500,
        .after_delay = 0,
        .speed_max = 64.0,
    },
    [12] = {
        .pos_target = 1.625,
        .after_delay = 0,
        .speed_max = 128.0,
    },
    [13] = {
        .pos_target = 1.750,
        .after_delay = 0,
        .speed_max = 128.0,
    },
    [14] = {
        .pos_target = 1.875,
        .after_delay = 0,
        .speed_max = 128.0,
    },
    [15] = {
        .pos_target = 2.0,
        .after_delay = 0,
        .speed_max = 256.0,
    },
    [16] = {
        .pos_target = 2.125,
        .after_delay = 0,
        .speed_max = 256.0,
    },
    [17] = {
        .pos_target = 2.250,
        .after_delay = 0,
        .speed_max = 256.0,
    },
    [18] = {
        .pos_target = 2.375,
        .after_delay = 0,
        .speed_max = 512.0,
    },
    [19] = {
        .pos_target = 2.500,
        .after_delay = 0,
        .speed_max = 1024.0,
    },
    [20] = {
        .pos_target = 2.625,
        .after_delay = 0,
        .speed_max = 1024.0,
    },
    [21] = {
        .pos_target = 2.750,
        .after_delay = 0,
        .speed_max = 2048.0,
    },
    [22] = {
        .pos_target = 2.875,
        .after_delay = 0,
        .speed_max = 2048.0,
    },
    [23] = {
        .pos_target = 3.0,
        .after_delay = 0,
        .speed_max = 2048.0,
    },
    [24] = {
        .pos_target = 3.125,
        .after_delay = 0,
        .speed_max = 2048.0,
    },
    [25] = {
        .pos_target = 3.250,
        .after_delay = 0,
        .speed_max = 2048.0,
    },
    [26] = {
        .pos_target = 3.375,
        .after_delay = 0,
        .speed_max = 2048.0,
    },
    [27] = {
        .pos_target = 3.5,
        .after_delay = 0,
        .speed_max = 2048.0,
    },
    [28] = {
        .pos_target = 3.625,
        .after_delay = 0,
        .speed_max = 2048.0,
    },
    [29] = {
        .pos_target = 3.750,
        .after_delay = 0,
        .speed_max = 2048.0,
    },
    [30] = {
        .pos_target = 3.875,
        .after_delay = 0,
        .speed_max = 2048.0,
    },
    [31] = {
        .pos_target = 4.0,
        .after_delay = 0,
        .speed_max = 2048.0,
    },
    [32] = {
        .pos_target = 4.125,
        .after_delay = 0,
        .speed_max = 2048.0,
    },
    [33] = {
        .pos_target = 4.250,
        .after_delay = 0,
        .speed_max = 2048.0,
    },
    [34] = {
        .pos_target = 4.375,
        .after_delay = 0,
        .speed_max = 2048.0,
    },
    [35] = {
        .pos_target = 4.500,
        .after_delay = 0,
        .speed_max = 2048.0,
    },
    [36] = {
        .pos_target = 4.625,
        .after_delay = 0,
        .speed_max = 2048.0,
    },
    [37] = {
        .pos_target = 4.750,
        .after_delay = 0,
        .speed_max = 2048.0,
    },
    [38] = {
        .pos_target = 4.875,
        .after_delay = 0,
        .speed_max = 2048.0,
    },
    [39] = {
        .pos_target = 5.0,
        .after_delay = 0,
        .speed_max = 2048.0,
    },
    [40] = {
        .pos_target = 5.125,
        .after_delay = 0,
        .speed_max = 4000.0,
    },
    [41] = {
        .pos_target = 5.250,
        .after_delay = 0,
        .speed_max = 4000.0,
    },
    [42] = {
        .pos_target = 5.375,
        .after_delay = 0,
        .speed_max = 4000.0,
    },
    [43] = {
        .pos_target = 5.500,
        .after_delay = 0,
        .speed_max = 4000.0,
    },
    [44] = {
        .pos_target = 5.625,
        .after_delay = 0,
        .speed_max = 4000.0,
    },
    [45] = {
        .pos_target = 5.750,
        .after_delay = 0,
        .speed_max = 4000.0,
    },
    [46] = {
        .pos_target = 5.875,
        .after_delay = 0,
        .speed_max = 4000.0,
    },
    [47] = {
        .pos_target = 6.0,
        .after_delay = 0,
        .speed_max = 4000.0,
    },
    [48] = {
        .pos_target = 6.125,
        .after_delay = 0,
        .speed_max = 4000.0,
    },
    [49] = {
        .pos_target = 6.250,
        .after_delay = 0,
        .speed_max = 4000.0,
    },
    [50] = {
        .pos_target = 6.375,
        .after_delay = 0,
        .speed_max = 4000.0,
    },
    [51] = {
        .pos_target = 6.500,
        .after_delay = 0,
        .speed_max = 4000.0,
    },
    [52] = {
        .pos_target = 6.625,
        .after_delay = 0,
        .speed_max = 4000.0,
    },
    [53] = {
        .pos_target = 6.750,
        .after_delay = 0,
        .speed_max = 4000.0,
    },
    [54] = {
        .pos_target = 6.875,
        .after_delay = 0,
        .speed_max = 4000.0,
    },
    [55] = {
        .pos_target = 7.000,
        .after_delay = 0,
        .speed_max = 4000.0,
    },
    [56] = {
        .pos_target = 7.125,
        .after_delay = 0,
        .speed_max = 4000.0,
    },
    [57] = {
        .pos_target = 7.250,
        .after_delay = 0,
        .speed_max = 4000.0,
    },
    [58] = {
        .pos_target = 7.375,
        .after_delay = 0,
        .speed_max = 4000.0,
    },
    [59] = {
        .pos_target = 7.500,
        .after_delay = 0,
        .speed_max = 4000.0,
    },
    [60] = {
        .pos_target = 7.625,
        .after_delay = 0,
        .speed_max = 4000.0,
    },
    [61] = {
        .pos_target = 7.750,
        .after_delay = 0,
        .speed_max = 4000.0,
    },
    [62] = {
        .pos_target = 7.875,
        .after_delay = 0,
        .speed_max = 4000.0,
    },
    [63] = {
        .pos_target = 8.000,
        .after_delay = 0,
        .speed_max = 4000.0,
    },
    [64] = {
        .pos_target = 8.125,
        .after_delay = 0,
        .speed_max = 4000.0,
    },
    [65] = {
        .pos_target = 8.250,
        .after_delay = 0,
        .speed_max = 4000.0,
    },
    [66] = {
        .pos_target = 8.375,
        .after_delay = 0,
        .speed_max = 4000.0,
    },
    [67] = {
        .pos_target = 8.500,
        .after_delay = 0,
        .speed_max = 4000.0,
    },
    [68] = {
        .pos_target = 8.625,
        .after_delay = 0,
        .speed_max = 4000.0,
    },
    [69] = {
        .pos_target = 8.750,
        .after_delay = 0,
        .speed_max = 4000.0,
    },
    [70] = {
        .pos_target = 8.875,
        .after_delay = 0,
        .speed_max = 4000.0,
    },
    [71] = {
        .pos_target = 9.0,
        .after_delay = 0,
        .speed_max = 4000.0,
    },
    [72] = {
        .pos_target = 9.125,
        .after_delay = 0,
        .speed_max = 4000.0,
    },
    [73] = {
        .pos_target = 9.250,
        .after_delay = 0,
        .speed_max = 4000.0,
    },
    [74] = {
        .pos_target = 9.375,
        .after_delay = 0,
        .speed_max = 4000.0,
    },
    [75] = {
        .pos_target = 9.500,
        .after_delay = 0,
        .speed_max = 4000.0,
    },
    [76] = {
        .pos_target = 9.625,
        .after_delay = 0,
        .speed_max = 4000.0,
    },
    [77] = {
        .pos_target = 9.750,
        .after_delay = 0,
        .speed_max = 4000.0,
    },
    [78] = {
        .pos_target = 9.875,
        .after_delay = 0,
        .speed_max = 4000.0,
    },
    [79] = {
        .pos_target = 10.000,
        .after_delay = 0,
        .speed_max = 4000.0,
    },
    [80] = {
        .pos_target = 10.125,
        .after_delay = 0,
        .speed_max = 4000.0,
    },
    [81] = {
        .pos_target = 10.250,
        .after_delay = 0,
        .speed_max = 4000.0,
    },
    [82] = {
        .pos_target = 10.375,
        .after_delay = 0,
        .speed_max = 4000.0,
    },
    [83] = {
        .pos_target = 10.500,
        .after_delay = 0,
        .speed_max = 4000.0,
    },
    [84] = {
        .pos_target = 10.625,
        .after_delay = 0,
        .speed_max = 4000.0,
    },
    [85] = {
        .pos_target = 10.750,
        .after_delay = 0,
        .speed_max = 4000.0,
    },
    [86] = {
        .pos_target = 10.875,
        .after_delay = 0,
        .speed_max = 4000.0,
    },
    [87] = {
        .pos_target = 11.000,
        .after_delay = 0,
        .speed_max = 4000.0,
    },
    [88] = {
        .pos_target = 11.125,
        .after_delay = 0,
        .speed_max = 4000.0,
    },
    [89] = {
        .pos_target = 11.250,
        .after_delay = 0,
        .speed_max = 4000.0,
    },
    [90] = {
        .pos_target = 11.375,
        .after_delay = 0,
        .speed_max = 4000.0,
    },
    [91] = {
        .pos_target = 11.500,
        .after_delay = 0,
        .speed_max = 4000.0,
    },
    [92] = {
        .pos_target = 11.625,
        .after_delay = 0,
        .speed_max = 4000.0,
    },
    [93] = {
        .pos_target = 11.750,
        .after_delay = 0,
        .speed_max = 4000.0,
    },
    [94] = {
        .pos_target = 11.875,
        .after_delay = 0,
        .speed_max = 4000.0,
    },
    [95] = {
        .pos_target = 12.000,
        .after_delay = 0,
        .speed_max = 4000.0,
    },
    [96] = {
        .pos_target = 12.125,
        .after_delay = 0,
        .speed_max = 4000.0,
    },
    [97] = {
        .pos_target = 12.250,
        .after_delay = 0,
        .speed_max = 4000.0,
    },
    [98] = {
        .pos_target = 12.375,
        .after_delay = 0,
        .speed_max = 4000.0,
    },
    [99] = {
        .pos_target = 12.500,
        .after_delay = 0,
        .speed_max = 4000.0,
    },
    [100] = {
        .pos_target = 12.625,
        .after_delay = 0,
        .speed_max = 4000.0,
    },
    [101] = {
        .pos_target = 12.750,
        .after_delay = 0,
        .speed_max = 4000.0,
    },
    [102] = {
        .pos_target = 12.875,
        .after_delay = 0,
        .speed_max = 4000.0,
    },
    [103] = {
        .pos_target = 13.000,
        .after_delay = 0,
        .speed_max = 4000.0,
    },
    [104] = {
        .pos_target = 13.125,
        .after_delay = 0,
        .speed_max = 4000.0,
    },
    [105] = {
        .pos_target = 13.250,
        .after_delay = 0,
        .speed_max = 4000.0,
    },
    [106] = {
        .pos_target = 13.375,
        .after_delay = 0,
        .speed_max = 4000.0,
    },
    [107] = {
        .pos_target = 13.500,
        .after_delay = 0,
        .speed_max = 4000.0,
    },
    [108] = {
        .pos_target = 13.625,
        .after_delay = 0,
        .speed_max = 4000.0,
    },
    [109] = {
        .pos_target = 13.750,
        .after_delay = 0,
        .speed_max = 4000.0,
    },
    [110] = {
        .pos_target = 13.875,
        .after_delay = 0,
        .speed_max = 4000.0,
    },
    [111] = {
        .pos_target = 14.000,
        .after_delay = 0,
        .speed_max = 4000.0,
    },
    [112] = {
        .pos_target = 14.125,
        .after_delay = 0,
        .speed_max = 4000.0,
    },
    [113] = {
        .pos_target = 14.250,
        .after_delay = 0,
        .speed_max = 4000.0,
    },
    [114] = {
        .pos_target = 14.375,
        .after_delay = 0,
        .speed_max = 4000.0,
    },
    [115] = {
        .pos_target = 14.500,
        .after_delay = 0,
        .speed_max = 4000.0,
    },
    [116] = {
        .pos_target = 14.625,
        .after_delay = 0,
        .speed_max = 4000.0,
    },
    [117] = {
        .pos_target = 14.750,
        .after_delay = 0,
        .speed_max = 4000.0,
    },
    [118] = {
        .pos_target = 14.875,
        .after_delay = 0,
        .speed_max = 4000.0,
    },
    [119] = {
        .pos_target = 15.000,
        .after_delay = 0,
        .speed_max = 4000.0,
    },
    [120] = {
        .pos_target = 15.125,
        .after_delay = 0,
        .speed_max = 4000.0,
    },
    [121] = {
        .pos_target = 15.250,
        .after_delay = 0,
        .speed_max = 4000.0,
    },
    [122] = {
        .pos_target = 15.375,
        .after_delay = 0,
        .speed_max = 4000.0,
    },
    [123] = {
        .pos_target = 15.500,
        .after_delay = 0,
        .speed_max = 4000.0,
    },
    [124] = {
        .pos_target = 15.625,
        .after_delay = 0,
        .speed_max = 4000.0,
    },
    [125] = {
        .pos_target = 15.750,
        .after_delay = 0,
        .speed_max = 4000.0,
    },
    [126] = {
        .pos_target = 15.875,
        .after_delay = 0,
        .speed_max = 4000.0,
    },
    [127] = {
        .pos_target = 16.000,
        .after_delay = 0,
        .speed_max = 4000.0,
    },
    [128] = {
        .pos_target = 16.125,
        .after_delay = 0,
        .speed_max = 4000.0,
    },
    [129] = {
        .pos_target = 16.250,
        .after_delay = 0,
        .speed_max = 4000.0,
    },
    [130] = {
        .pos_target = 16.375,
        .after_delay = 0,
        .speed_max = 4000.0,
    },
    [131] = {
        .pos_target = 16.500,
        .after_delay = 0,
        .speed_max = 4000.0,
    },
    [132] = {
        .pos_target = 16.625,
        .after_delay = 0,
        .speed_max = 4000.0,
    },
    [133] = {
        .pos_target = 16.750,
        .after_delay = 0,
        .speed_max = 4000.0,
    },
    [134] = {
        .pos_target = 16.875,
        .after_delay = 0,
        .speed_max = 4000.0,
    },
    [135] = {
        .pos_target = 17.000,
        .after_delay = 0,
        .speed_max = 4000.0,
    },
    [136] = {
        .pos_target = 17.125,
        .after_delay = 0,
        .speed_max = 4000.0,
    },
    [137] = {
        .pos_target = 17.250,
        .after_delay = 0,
        .speed_max = 4000.0,
    },
    [138] = {
        .pos_target = 17.375,
        .after_delay = 0,
        .speed_max = 4000.0,
    },
    [139] = {
        .pos_target = 17.500,
        .after_delay = 0,
        .speed_max = 4000.0,
    },
    [140] = {
        .pos_target = 17.625,
        .after_delay = 0,
        .speed_max = 4000.0,
    },
    [141] = {
        .pos_target = 17.750,
        .after_delay = 0,
        .speed_max = 4000.0,
    },
    [142] = {
        .pos_target = 17.875,
        .after_delay = 0,
        .speed_max = 4000.0,
    },
    [143] = {
        .pos_target = 18.000,
        .after_delay = 0,
        .speed_max = 4000.0,
    },
    [144] = {
        .pos_target = 17.000,
        .after_delay = 0,
        .speed_max = 4000.0,
    },
    [145] = {
        .pos_target = 16.250,
        .after_delay = 0,
        .speed_max = 4000.0,
    },
    [146] = {
        .pos_target = 16.125,
        .after_delay = 0,
        .speed_max = 4000.0,
    },
    [147] = {
        .pos_target = 15.500,
        .after_delay = 0,
        .speed_max = 4000.0,
    },
    [148] = {
        .pos_target = 15.000,
        .after_delay = 0,
        .speed_max = 4000.0,
    },
    [149] = {
        .pos_target = 14.750,
        .after_delay = 0,
        .speed_max = 4000.0,
    },
    [150] = {
        .pos_target = 14.375,
        .after_delay = 0,
        .speed_max = 4000.0,
    },
    [151] = {
        .pos_target = 14.000,
        .after_delay = 0,
        .speed_max = 4000.0,
    },
    [152] = {
        .pos_target = 13.500,
        .after_delay = 0,
        .speed_max = 4000.0,
    },
    [153] = {
        .pos_target = 13.000,
        .after_delay = 0,
        .speed_max = 4000.0,
    },
    [154] = {
        .pos_target = 12.000,
        .after_delay = 0,
        .speed_max = 4000.0,
    },
    [155] = {
        .pos_target = 11.500,
        .after_delay = 0,
        .speed_max = 4000.0,
    },
    [156] = {
        .pos_target = 11.250,
        .after_delay = 0,
        .speed_max = 4000.0,
    },
    [157] = {
        .pos_target = 11.000,
        .after_delay = 0,
        .speed_max = 4000.0,
    },
    [158] = {
        .pos_target = 10.625,
        .after_delay = 0,
        .speed_max = 4000.0,
    },
    [159] = {
        .pos_target = 10.500,
        .after_delay = 0,
        .speed_max = 4000.0,
    },
    [160] = {
        .pos_target = 10.000,
        .after_delay = 0,
        .speed_max = 4000.0,
    },
    [161] = {
        .pos_target = 9.500,
        .after_delay = 0,
        .speed_max = 4000.0,
    },
    [162] = {
        .pos_target = 9.250,
        .after_delay = 0,
        .speed_max = 4000.0,
    },
    [163] = {
        .pos_target = 8.750,
        .after_delay = 0,
        .speed_max = 4000.0,
    },
    [164] = {
        .pos_target = 8.250,
        .after_delay = 0,
        .speed_max = 4000.0,
    },
    [165] = {
        .pos_target = 8.000,
        .after_delay = 0,
        .speed_max = 4000.0,
    },
    [166] = {
        .pos_target = 7.625,
        .after_delay = 0,
        .speed_max = 4000.0,
    },
    [167] = {
        .pos_target = 7.500,
        .after_delay = 0,
        .speed_max = 4000.0,
    },
    [168] = {
        .pos_target = 7.000,
        .after_delay = 0,
        .speed_max = 4000.0,
    },
    [169] = {
        .pos_target = 6.500,
        .after_delay = 0,
        .speed_max = 4000.0,
    },
    [170] = {
        .pos_target = 5.500,
        .after_delay = 0,
        .speed_max = 4000.0,
    },
    [171] = {
        .pos_target = 5.250,
        .after_delay = 0,
        .speed_max = 4000.0,
    },
    [172] = {
        .pos_target = 5.125,
        .after_delay = 0,
        .speed_max = 4000.0,
    },
    [173] = {
        .pos_target = 4.250,
        .after_delay = 0,
        .speed_max = 4000.0,
    },
    [174] = {
        .pos_target = 3.875,
        .after_delay = 0,
        .speed_max = 4000.0,
    },
    [175] = {
        .pos_target = 3.625,
        .after_delay = 0,
        .speed_max = 4000.0,
    },
    [176] = {
        .pos_target = 3.500,
        .after_delay = 0,
        .speed_max = 4000.0,
    },
    [177] = {
        .pos_target = 3.250,
        .after_delay = 0,
        .speed_max = 4000.0,
    },
    [178] = {
        .pos_target = 3.125,
        .after_delay = 0,
        .speed_max = 4000.0,
    },
    [179] = {
        .pos_target = 3.0,
        .after_delay = 0,
        .speed_max = 4000.0,
    },
    [180] = {
        .pos_target = 2.5,
        .after_delay = 0,
        .speed_max = 4000.0,
    },
    [181] = {
        .pos_target = 3.0,
        .after_delay = 0,
        .speed_max = 4000.0,
    },
    [182] = {
        .pos_target = 2.5,
        .after_delay = 0,
        .speed_max = 4000.0,
    },
    [183] = {
        .pos_target = 2.0,
        .after_delay = 0,
        .speed_max = 4000.0,
    },
    [184] = {
        .pos_target = 1.750,
        .after_delay = 0,
        .speed_max = 4000.0,
    },
    [185] = {
        .pos_target = 1.500,
        .after_delay = 0,
        .speed_max = 4000.0,
    },
    [186] = {
        .pos_target = 1.250,
        .after_delay = 0,
        .speed_max = 4000.0,
    },
    [187] = {
        .pos_target = 1.0,
        .after_delay = 0,
        .speed_max = 4000.0,
    },
    [188] = {
        .pos_target = 0.750,
        .after_delay = 0,
        .speed_max = 4000.0,
    },
    [189] = {
        .pos_target = 0.5,
        .after_delay = 0,
        .speed_max = 4000.0,
    },
    [190] = {
        .pos_target = 0.250,
        .after_delay = 0,
        .speed_max = 4000.0,
    },
    [191] = {
        .pos_target = 0.0,
        .after_delay = 5000,
        .speed_max = 4000.0,
    },
};

struct scenario_block blocks_test[] = {
    // Short moves at slow speed
    [0] = {
        .pos_target = 0.125,
        .after_delay = 100,
        .speed_max = 15.0,
    },
    [1] = {
        .pos_target = 0.250,
        .after_delay = 100,
        .speed_max = 15.0,
    },
    [2] = {
        .pos_target = 0.375,
        .after_delay = 100,
        .speed_max = 15.0,
    },
    [3] = {
        .pos_target = 0.500,
        .after_delay = 100,
        .speed_max = 15.0,
    },
    [4] = {
        .pos_target = 0.625,
        .after_delay = 100,
        .speed_max = 15.0,
    },
    [6] = {
        .pos_target = 0.750,
        .after_delay = 100,
        .speed_max = 15.0,
    },
    [7] = {
        .pos_target = 0.875,
        .after_delay = 100,
        .speed_max = 15.0,
    },
    [8] = {
        .pos_target = 1.0,
        .after_delay = 100,
        .speed_max = 15.0,
    },
    [9] = {
        .pos_target = 1.250,
        .after_delay = 100,
        .speed_max = 15.0,
    },
    [10] = {
        .pos_target = 1.500,
        .after_delay = 100,
        .speed_max = 15.0,
    },
    [11] = {
        .pos_target = 1.750,
        .after_delay = 100,
        .speed_max = 15.0,
    },
    [12] = {
        .pos_target = 2.0,
        .after_delay = 100,
        .speed_max = 15.0,
    },
    [13] = {
        .pos_target = 2.500,
        .after_delay = 100,
        .speed_max = 15.0,
    },
    [14] = {
        .pos_target = 3.0,
        .after_delay = 100,
        .speed_max = 15.0,
    },
    // Short moves at medium speed
    [15] = {
        .pos_target = 3.125,
        .after_delay = 100,
        .speed_max = 100.0,
    },
    [16] = {
        .pos_target = 3.250,
        .after_delay = 100,
        .speed_max = 100.0,
    },
    [17] = {
        .pos_target = 3.375,
        .after_delay = 100,
        .speed_max = 100.0,
    },
    [18] = {
        .pos_target = 3.500,
        .after_delay = 0,
        .speed_max = 100.0,
    },
    [19] = {
        .pos_target = 3.625,
        .after_delay = 100,
        .speed_max = 100.0,
    },
    [20] = {
        .pos_target = 3.750,
        .after_delay = 100,
        .speed_max = 100.0,
    },
    [21] = {
        .pos_target = 3.875,
        .after_delay = 100,
        .speed_max = 100.0,
    },
    [22] = {
        .pos_target = 4.0,
        .after_delay = 100,
        .speed_max = 100.0,
    },
    [23] = {
        .pos_target = 4.250,
        .after_delay = 100,
        .speed_max = 100.0,
    },
    [24] = {
        .pos_target = 4.500,
        .after_delay = 100,
        .speed_max = 100.0,
    },
    [25] = {
        .pos_target = 4.750,
        .after_delay = 100,
        .speed_max = 100.0,
    },
    [26] = {
        .pos_target = 5.000,
        .after_delay = 100,
        .speed_max = 100.0,
    },
    [27] = {
        .pos_target = 5.500,
        .after_delay = 100,
        .speed_max = 100.0,
    },
    [28] = {
        .pos_target = 6.000,
        .after_delay = 100,
        .speed_max = 100.0,
    },
    // Short moves at high speed
    [29] = {
        .pos_target = 6.125,
        .after_delay = 100,
        .speed_max = 500.0,
    },
    [30] = {
        .pos_target = 6.250,
        .after_delay = 100,
        .speed_max = 500.0,
    },
    [31] = {
        .pos_target = 6.375,
        .after_delay = 100,
        .speed_max = 500.0,
    },
    [32] = {
        .pos_target = 6.500,
        .after_delay = 100,
        .speed_max = 500.0,
    },
    [33] = {
        .pos_target = 6.625,
        .after_delay = 100,
        .speed_max = 500.0,
    },
    [34] = {
        .pos_target = 6.750,
        .after_delay = 100,
        .speed_max = 500.0,
    },
    [35] = {
        .pos_target = 6.875,
        .after_delay = 100,
        .speed_max = 500.0,
    },
    [36] = {
        .pos_target = 7.0,
        .after_delay = 100,
        .speed_max = 500.0,
    },
    [37] = {
        .pos_target = 7.250,
        .after_delay = 100,
        .speed_max = 500.0,
    },
    [38] = {
        .pos_target = 7.500,
        .after_delay = 100,
        .speed_max = 500.0,
    },
    [39] = {
        .pos_target = 7.750,
        .after_delay = 100,
        .speed_max = 500.0,
    },
    [40] = {
        .pos_target = 8.000,
        .after_delay = 100,
        .speed_max = 500.0,
    },
    [41] = {
        .pos_target = 8.500,
        .after_delay = 100,
        .speed_max = 500.0,
    },
    [42] = {
        .pos_target = 9.000,
        .after_delay = 1000,
        .speed_max = 500.0,
    },
    // Long move at slow speed
    [43] = {
        .pos_target = 14.000,
        .after_delay = 500,
        .speed_max = 500.0,
    },
    // Long move at medium speed
    [44] = {
        .pos_target = 24.000,
        .after_delay = 1000,
        .speed_max = 1000.0,
    },
    [45] = {
        .pos_target = 0.000,
        .after_delay = 1000,
        .speed_max = 1000.0,
    },
};

void get_scenario_block(struct scenario_ctx *ctx)
{
    if (ctx->blocks != NULL) {
        ctx->pos_target = ctx->blocks[ctx->blocks_iterator].pos_target;
        ctx->speed_max = ctx->blocks[ctx->blocks_iterator].speed_max;
        //FIXME change after_delay values to time to avoid this scaling
        ctx->after_delay = (ctx->blocks[ctx->blocks_iterator].after_delay * APP_PERIOD_250_US) / ctx->app_period_ns;
    } else {
        log_err("Scenario blocks pointer is NULL\n");
    }
}

void next_block_scenario(struct scenario_ctx *ctx)
{
    ctx->blocks_iterator++;
    ctx->blocks_iterator %= ctx->scenario_size;
}

int scenario_init(struct scenario_ctx *ctx, int id_scenario, unsigned int app_period_ns)
{
    ctx->blocks_iterator = 0;
    ctx->pos_target = 0.0;
    ctx->speed_max = 0.0;
    ctx->app_period_ns = app_period_ns;

    switch (id_scenario) {
    case SCENARIO_SYNCHRONIZED:
        ctx->blocks = blocks_0;
        ctx->scenario_size = sizeof(blocks_0) / sizeof(blocks_0[0]);
        break;
    case SCENARIO_SYNCHRONIZED_REVERSE:
        ctx->blocks = blocks_0_rev;
        ctx->scenario_size = sizeof(blocks_0_rev) / sizeof(blocks_0_rev[0]);
        break;
    case SCENARIO_INTERLACED:
        ctx->blocks = blocks_2;
        ctx->scenario_size = sizeof(blocks_2) / sizeof(blocks_2[0]);
        break;
    case SCENARIO_TEST:
        ctx->blocks = blocks_test;
        ctx->scenario_size = sizeof(blocks_test) / sizeof(blocks_test[0]);
        break;
    default:
        goto err;
        break;
    }

    return 0;

err:
    return -1;
}

void reset_scenario(struct scenario_ctx *ctx)
{
    ctx->blocks = NULL;
    ctx->blocks_iterator = 0;
    ctx->pos_target = 0.0;
    ctx->speed_max = 0.0;
    ctx->after_delay = 0;
    ctx->time_to_reach = 0;
    ctx->scenario_size = 0;
}
