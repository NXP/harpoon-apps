/*
 * Copyright 2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_clock.h"
#include "audio_pipeline.h"

const struct audio_pipeline_config pipeline_dtmf_config = {

	.name = "DTMF pipeline",

	.stages = 2,

	.stage[0] = {

		.elements = 4,

		.element[0] = {
			.type = AUDIO_ELEMENT_DTMF_SOURCE,
			.u.dtmf = {
				.us = 120000,
				.pause_us = 100000,
				.sequence_pause_us = 500000,
				.amplitude = 0.5,
				.sequence = "1123ABCD0123456789*#",
			},

			.outputs = 1,
			.output = {0},
		},

		.element[1] = {
			.type = AUDIO_ELEMENT_DTMF_SOURCE,
			.u.dtmf = {
				.us = 120000,
				.pause_us = 100000,
				.sequence_pause_us = 500000,
				.amplitude = 0.5,
				.sequence = "#*9876543210DCBA3211",
			},

			.outputs = 1,
			.output = {1},
		},

		.element[2] = {
			.type = AUDIO_ELEMENT_DTMF_SOURCE,
			.u.dtmf = {
				.us = 120000,
				.pause_us = 100000,
				.sequence_pause_us = 500000,
				.amplitude = 0.5,
				.sequence = "1123ABCD0123456789*#",
			},

			.outputs = 1,
			.output = {2},
		},

		.element[3] = {
			.type = AUDIO_ELEMENT_DTMF_SOURCE,
			.u.dtmf = {
				.us = 120000,
				.pause_us = 100000,
				.sequence_pause_us = 500000,
				.amplitude = 0.5,
				.sequence = "#*9876543210DCBA3211",
			},

			.outputs = 1,
			.output = {3},
		},
	},

	.stage[1] = {

		.elements = 2,

		.element[0] = {
			.type = AUDIO_ELEMENT_SAI_SINK,
			.u.sai_sink = {
				.sai_n = 2,
				.sai = {
					[0] = {
						.id = 5,
						.line_n = 1,
						.line = {
							[0] = {
								.channel_n = 2,
							},
						},
					},
					[1] = {
						.id = 3,
						.line_n = 1,
						.line = {
							[0] = {
								.channel_n = 2,
							},
						},
					},
				},
			},

			.inputs = 4,
			.input = {0, 1, 2, 3},
		},

		.element[1] = {
			.type = AUDIO_ELEMENT_PLL,
			.u.pll = {
				.src_sai_id = 5,
				.dst_sai_id = 3,
				.pll_id = kCLOCK_AudioPll1Ctrl,
			},
		},
	},

	.buffers = 4,

	.buffer_storage = 4,
};


const struct audio_pipeline_config pipeline_loopback_config = {

	.name = "Loopback pipeline",

	.stages = 2,

	.stage[0] = {

		.elements = 1,

		.element[0] = {
			.type = AUDIO_ELEMENT_SAI_SOURCE,
			.u.sai_source = {
				.sai_n = 2,
				.sai = {
					[0] = {
						.id = 5,
						.line_n = 1,
						.line = {
							[0] = {
								.channel_n = 2,
							},
						},
					},
					[1] = {
						.id = 3,
						.line_n = 1,
						.line = {
							[0] = {
								.channel_n = 2,
							},
						},
					},
				},
			},
			.outputs = 4,
			.output = {0, },	/* 0 - 3 */
		},
	},

	.stage[1] = {

		.elements = 2,

		.element[0] = {
			.type = AUDIO_ELEMENT_SAI_SINK,
			.u.sai_sink = {
				.sai_n = 2,
				.sai = {
					[0] = {
						.id = 5,
						.line_n = 1,
						.line = {
							[0] = {
								.channel_n = 2,
							},
						},
					},
					[1] = {
						.id = 3,
						.line_n = 1,
						.line = {
							[0] = {
								.channel_n = 2,
							},
						},
					},
				},
			},

			.inputs = 4,
			.input = {0, },	/* 0 - 3 */
		},

		.element[1] = {
			.type = AUDIO_ELEMENT_PLL,
			.u.pll = {
				.src_sai_id = 5,
				.dst_sai_id = 3,
				.pll_id = kCLOCK_AudioPll1Ctrl,
			},
		},
	},

	.buffers = 4,

	.buffer_storage = 4,
};


const struct audio_pipeline_config pipeline_sine_config = {

	.name = "Sinewave pipeline",

	.stages = 2,

	.stage[0] = {

		.elements = 4,

		.element[0] = {
			.type = AUDIO_ELEMENT_SINE_SOURCE,
			.u.sine = {
				.freq = 440,
				.amplitude = 0.5,
			},
			.outputs = 1,
			.output = {0},
		},

		.element[1] = {
			.type = AUDIO_ELEMENT_SINE_SOURCE,
			.u.sine = {
				.freq = 440,
				.amplitude = 0.5,
			},
			.outputs = 1,
			.output = {1},
		},

		.element[2] = {
			.type = AUDIO_ELEMENT_SINE_SOURCE,
			.u.sine = {
				.freq = 440,
				.amplitude = 0.5,
			},
			.outputs = 1,
			.output = {2},
		},

		.element[3] = {
			.type = AUDIO_ELEMENT_SINE_SOURCE,
			.u.sine = {
				.freq = 440,
				.amplitude = 0.5,
			},
			.outputs = 1,
			.output = {3},
		}
	},

	.stage[1] = {

		.elements = 2,

		.element[0] = {
			.type = AUDIO_ELEMENT_SAI_SINK,
			.u.sai_sink = {
				.sai_n = 2,
				.sai = {
					[0] = {
						.id = 5,
						.line_n = 1,
						.line = {
							[0] = {
								.channel_n = 2,
							},
						},
					},
					[1] = {
						.id = 3,
						.line_n = 1,
						.line = {
							[0] = {
								.channel_n = 2,
							},
						},
					},
				},
			},

			.inputs = 4,
			.input = {0, 1, 2, 3},
		},

		.element[1] = {
			.type = AUDIO_ELEMENT_PLL,
			.u.pll = {
				.src_sai_id = 5,
				.dst_sai_id = 3,
				.pll_id = kCLOCK_AudioPll1Ctrl,
			},
		},
	},

	.buffers = 4,

	.buffer_storage = 4,
};

const struct audio_pipeline_config pipeline_full_config = {

	.name = "Full audio pipeline",

	.stages = 3,

	.stage[0] = {

		.elements = 5,

		.element[0] = {
			.type = AUDIO_ELEMENT_DTMF_SOURCE,
			.u.dtmf = {
				.us = 120000,
				.pause_us = 100000,
				.sequence_pause_us = 500000,
				.amplitude = 0.5,
				.sequence = "1123ABCD0123456789*#",
			},

			.outputs = 1,
			.output = {0},
		},

		.element[1] = {
			.type = AUDIO_ELEMENT_DTMF_SOURCE,
			.u.dtmf = {
				.us = 120000,
				.pause_us = 100000,
				.sequence_pause_us = 500000,
				.amplitude = 0.5,
				.sequence = "#*9876543210DCBA3211",
			},

			.outputs = 1,
			.output = {1},
		},

		.element[2] = {
			.type = AUDIO_ELEMENT_SINE_SOURCE,
			.u.sine = {
				.freq = 440,
				.amplitude = 0.5,
			},
			.outputs = 1,
			.output = {2},
		},

		.element[3] = {
			.type = AUDIO_ELEMENT_SINE_SOURCE,
			.u.sine = {
				.freq = 880,
				.amplitude = 0.5,
			},
			.outputs = 1,
			.output = {3},
		},

		.element[4] = {
			.type = AUDIO_ELEMENT_SAI_SOURCE,
			.u.sai_source = {
				.sai_n = 2,
				.sai = {
					[0] = {
						.id = 5,
						.line_n = 1,
						.line = {
							[0] = {
								.channel_n = 2,
							},
						},
					},
					[1] = {
						.id = 3,
						.line_n = 1,
						.line = {
							[0] = {
								.channel_n = 2,
							},
						},
					},
				},
			},
			.outputs = 4,
			.output = {4, },	/* 4 - 7 */
		},
	},

	.stage[1] = {

		.elements = 1,

		.element[0] = {
			.type = AUDIO_ELEMENT_ROUTING,
			.u.routing = {

			},

			.inputs = 8,
			.input = {0, }, 	/* 0 - 7 */

			.outputs = 4,
			.output = {8, },	/* 8 - 11 */
		},
	},

	.stage[2] = {

		.elements = 2,

		.element[0] = {
			.type = AUDIO_ELEMENT_SAI_SINK,
			.u.sai_sink = {
				.sai_n = 2,
				.sai = {
					[0] = {
						.id = 5,
						.line_n = 1,
						.line = {
							[0] = {
								.channel_n = 2,
							},
						},
					},
					[1] = {
						.id = 3,
						.line_n = 1,
						.line = {
							[0] = {
								.channel_n = 2,
							},
						},
					},
				},
			},

			.inputs = 4,
			.input = {8, },	/* 8 - 11 */
		},

		.element[1] = {
			.type = AUDIO_ELEMENT_PLL,
			.u.pll = {
				.src_sai_id = 5,
				.dst_sai_id = 3,
				.pll_id = kCLOCK_AudioPll1Ctrl,
			},
		},
	},

	.buffers = 12,

	.buffer_storage = 12,
};

#if (CONFIG_GENAVB_ENABLE == 1)
const struct audio_pipeline_config pipeline_full_avb_config = {

	.name = "AVB audio pipeline",

	.stages = 3,

	.stage[0] = {

		.elements = 3,

		.element[0] = {
			.type = AUDIO_ELEMENT_SINE_SOURCE,
			.u.sine = {
				.freq = 440,
				.amplitude = 0.5,
			},
			.outputs = 1,
			.output = {0},
		},

		.element[1] = {
			.type = AUDIO_ELEMENT_SAI_SOURCE,
			.u.sai_source = {
				.sai_n = 2,
				.sai = {
					[0] = {
						.id = 5,
						.line_n = 1,
						.line = {
							[0] = {
								.channel_n = 2,
							},
						},
					},
					[1] = {
						.id = 3,
						.line_n = 1,
						.line = {
							[0] = {
								.channel_n = 2,
							},
						},
					},
				},
			},
			.outputs = 4,
			.output = {1, },	/* 1 - 4 */
		},

		.element[2] = {
			.type = AUDIO_ELEMENT_AVTP_SOURCE,
			.u.avtp_source = {
				.stream_n = 2,
			},
			.outputs = 4,
			.output = {5, },	/* 5 - 8 */
		},
	},

	.stage[1] = {

		.elements = 1,

		.element[0] = {
			.type = AUDIO_ELEMENT_ROUTING,
			.u.routing = {

			},

			.inputs = 9,
			.input = {0, }, 	/* 0 - 8 */

			.outputs = 8,
			.output = {9, },	/* 9 - 12 */
		},
	},

	.stage[2] = {

		.elements = 3,

		.element[0] = {
			.type = AUDIO_ELEMENT_SAI_SINK,
			.u.sai_sink = {
				.sai_n = 2,
				.sai = {
					[0] = {
						.id = 5,
						.line_n = 1,
						.line = {
							[0] = {
								.channel_n = 2,
							},
						},
					},
					[1] = {
						.id = 3,
						.line_n = 1,
						.line = {
							[0] = {
								.channel_n = 2,
							},
						},
					},
				},
			},

			.inputs = 4,
			.input = {9, },	/* 9 - 12 */
		},

		.element[1] = {
			.type = AUDIO_ELEMENT_AVTP_SINK,
			.u.avtp_sink = {
				.stream_n = 2,
			},

			.inputs = 4,
			.input = {13, },	/* 13 - 16 */
		},

	.element[2] = {
			.type = AUDIO_ELEMENT_PLL,
			.u.pll = {
				.src_sai_id = 5,
				.dst_sai_id = 3,
				.pll_id = kCLOCK_AudioPll1Ctrl,
			},
		},
	},

	.buffers = 17,

	.buffer_storage = 17,
	.storage = {
		[5] = {.periods = AUDIO_PIPELINE_AVB_MAX_BUFFER_SIZE},
		[6] = {.periods = AUDIO_PIPELINE_AVB_MAX_BUFFER_SIZE},
		[7] = {.periods = AUDIO_PIPELINE_AVB_MAX_BUFFER_SIZE},
		[8] = {.periods = AUDIO_PIPELINE_AVB_MAX_BUFFER_SIZE},
	},
};
#endif /* #if (CONFIG_GENAVB_ENABLE == 1) */

#if defined(CONFIG_SMP) && (CONFIG_GENAVB_ENABLE == 1)

const struct audio_pipeline_config pipeline_full_avb_thread_0_config = {

	.name = "AVB audio pipeline for thread 0",

	.stages = 2,

	.stage[0] = {

		.elements = 3,

		.element[0] = {
			.type = AUDIO_ELEMENT_SINE_SOURCE,
			.u.sine = {
				.freq = 440,
				.amplitude = 0.5,
			},
			.outputs = 1,
			.output = {0},
		},

		.element[1] = {
			.type = AUDIO_ELEMENT_SAI_SOURCE,
			.u.sai_source = {
				.sai_n = 2,
				.sai = {
					[0] = {
						.id = 5,
						.line_n = 1,
						.line = {
							[0] = {
								.channel_n = 2,
							},
						},
					},
					[1] = {
						.id = 3,
						.line_n = 1,
						.line = {
							[0] = {
								.channel_n = 2,
							},
						},
					},
				},
			},
			.outputs = 4,
			.output = {1, },	/* 1 - 4 */
		},

		.element[2] = {
			.type = AUDIO_ELEMENT_AVTP_SOURCE,
			.u.avtp_source = {
				.stream_n = 2,
			},
			.outputs = 4,
			.output = {5, },	/* 5 - 8 */
		},
	},

	.stage[1] = {

		.elements = 1,

		.element[0] = {
			.type = AUDIO_ELEMENT_ROUTING,
			.u.routing = {

			},

			.inputs = 9,
			.input = {0, }, 	/* 0 - 8 */

			.outputs = 8,
			.output = {9, },	/* 9 - 17 */
		},
	},

	.buffers = 17,

	.buffer = {
		[9] = {.flags = AUDIO_BUFFER_FLAG_SHARED, .shared_id = 0},
		[10] = {.flags = AUDIO_BUFFER_FLAG_SHARED, .shared_id = 1},
		[11] = {.flags = AUDIO_BUFFER_FLAG_SHARED, .shared_id = 2},
		[12] = {.flags = AUDIO_BUFFER_FLAG_SHARED, .shared_id = 3},
		[13] = {.flags = AUDIO_BUFFER_FLAG_SHARED, .shared_id = 4},
		[14] = {.flags = AUDIO_BUFFER_FLAG_SHARED, .shared_id = 5},
		[15] = {.flags = AUDIO_BUFFER_FLAG_SHARED, .shared_id = 6},
		[16] = {.flags = AUDIO_BUFFER_FLAG_SHARED, .shared_id = 7},
	},
	.buffer_storage = 17,

	.storage = {
		[9] = {.periods = AUDIO_PIPELINE_AVB_MAX_BUFFER_SIZE},
		[10] = {.periods = AUDIO_PIPELINE_AVB_MAX_BUFFER_SIZE},
		[11] = {.periods = AUDIO_PIPELINE_AVB_MAX_BUFFER_SIZE},
		[12] = {.periods = AUDIO_PIPELINE_AVB_MAX_BUFFER_SIZE},
		[13] = {.periods = AUDIO_PIPELINE_AVB_MAX_BUFFER_SIZE},
		[14] = {.periods = AUDIO_PIPELINE_AVB_MAX_BUFFER_SIZE},
		[15] = {.periods = AUDIO_PIPELINE_AVB_MAX_BUFFER_SIZE},
		[16] = {.periods = AUDIO_PIPELINE_AVB_MAX_BUFFER_SIZE},
	},
};

const struct audio_pipeline_config pipeline_full_avb_thread_1_config = {

	.name = "AVB audio pipeline for thread 1",

	.stages = 1,

	.stage[0] = {

		.elements = 3,

		.element[0] = {
			.type = AUDIO_ELEMENT_SAI_SINK,
			.u.sai_sink = {
				.sai_n = 2,
				.sai = {
					[0] = {
						.id = 5,
						.line_n = 1,
						.line = {
							[0] = {
								.channel_n = 2,
							},
						},
					},
					[1] = {
						.id = 3,
						.line_n = 1,
						.line = {
							[0] = {
								.channel_n = 2,
							},
						},
					},
				},
			},

			.inputs = 4,
			.input = {0, },	/* 0 - 3 */
		},

		.element[1] = {
			.type = AUDIO_ELEMENT_AVTP_SINK,
			.u.avtp_sink = {
				.stream_n = 2,
			},

			.inputs = 4,
			.input = {4, },	/* 4 - 7 */
		},

	.element[2] = {
			.type = AUDIO_ELEMENT_PLL,
			.u.pll = {
				.src_sai_id = 5,
				.dst_sai_id = 3,
				.pll_id = kCLOCK_AudioPll1Ctrl,
			},
		},
	},

	.buffers = 8,

	.buffer = {
		[0] = { .flags = AUDIO_BUFFER_FLAG_SHARED_USER, .shared_id = 0},
		[1] = { .flags = AUDIO_BUFFER_FLAG_SHARED_USER, .shared_id = 1},
		[2] = { .flags = AUDIO_BUFFER_FLAG_SHARED_USER, .shared_id = 2},
		[3] = { .flags = AUDIO_BUFFER_FLAG_SHARED_USER, .shared_id = 3},
		[4] = { .flags = AUDIO_BUFFER_FLAG_SHARED_USER, .shared_id = 4},
		[5] = { .flags = AUDIO_BUFFER_FLAG_SHARED_USER, .shared_id = 5},
		[6] = { .flags = AUDIO_BUFFER_FLAG_SHARED_USER, .shared_id = 6},
		[7] = { .flags = AUDIO_BUFFER_FLAG_SHARED_USER, .shared_id = 7},
	},

	.buffer_storage = 0,
};

#endif /* defined(CONFIG_SMP) && (CONFIG_GENAVB_ENABLE == 1) */
#if defined(CONFIG_SMP)

const struct audio_pipeline_config pipeline_full_thread_0_config = {

	.name = "Full audio pipeline for thread 0",

	.stages = 2,

	.stage[0] = {

		.elements = 5,

		.element[0] = {
			.type = AUDIO_ELEMENT_DTMF_SOURCE,
			.u.dtmf = {
				.us = 120000,
				.pause_us = 100000,
				.sequence_pause_us = 500000,
				.amplitude = 0.5,
				.sequence = "1123ABCD0123456789*#",
			},

			.outputs = 1,
			.output = {0},
		},

		.element[1] = {
			.type = AUDIO_ELEMENT_DTMF_SOURCE,
			.u.dtmf = {
				.us = 120000,
				.pause_us = 100000,
				.sequence_pause_us = 500000,
				.amplitude = 0.5,
				.sequence = "#*9876543210DCBA3211",
			},

			.outputs = 1,
			.output = {1},
		},

		.element[2] = {
			.type = AUDIO_ELEMENT_SINE_SOURCE,
			.u.sine = {
				.freq = 440,
				.amplitude = 0.5,
			},
			.outputs = 1,
			.output = {2},
		},

		.element[3] = {
			.type = AUDIO_ELEMENT_SINE_SOURCE,
			.u.sine = {
				.freq = 880,
				.amplitude = 0.5,
			},
			.outputs = 1,
			.output = {3},
		},

		.element[4] = {
			.type = AUDIO_ELEMENT_SAI_SOURCE,
			.u.sai_source = {
				.sai_n = 2,
				.sai = {
					[0] = {
						.id = 5,
						.line_n = 1,
						.line = {
							[0] = {
								.channel_n = 2,
							},
						},
					},
					[1] = {
						.id = 3,
						.line_n = 1,
						.line = {
							[0] = {
								.channel_n = 2,
							},
						},
					},
				},
			},
			.outputs = 4,
			.output = {4, },	/* 4 - 7 */
		},
	},

	.stage[1] = {

		.elements = 1,

		.element[0] = {
			.type = AUDIO_ELEMENT_ROUTING,
			.u.routing = {

			},

			.inputs = 8,
			.input = {0, }, 	/* 0 - 7 */

			.outputs = 4,
			.output = {8, },	/* 8 - 11 */
		},
	},

	.buffers = 12,

	.buffer = {
		[8] = {.flags = AUDIO_BUFFER_FLAG_SHARED, .shared_id = 0},
		[9] = {.flags = AUDIO_BUFFER_FLAG_SHARED, .shared_id = 1},
		[10] = {.flags = AUDIO_BUFFER_FLAG_SHARED, .shared_id = 2},
		[11] = {.flags = AUDIO_BUFFER_FLAG_SHARED, .shared_id = 3},
	},
	.buffer_storage = 12,

	.storage = {
		[8] = {.periods = 3},
		[9] = {.periods = 3},
		[10] = {.periods = 3},
		[11] = {.periods = 3},
	},
};

const struct audio_pipeline_config pipeline_full_thread_1_config = {

	.name = "Full audio pipeline for thread 1",

	.stages = 1,

	.stage[0] = {

		.elements = 2,

		.element[0] = {
			.type = AUDIO_ELEMENT_SAI_SINK,
			.u.sai_sink = {
				.sai_n = 2,
				.sai = {
					[0] = {
						.id = 5,
						.line_n = 1,
						.line = {
							[0] = {
								.channel_n = 2,
							},
						},
					},
					[1] = {
						.id = 3,
						.line_n = 1,
						.line = {
							[0] = {
								.channel_n = 2,
							},
						},
					},
				},
			},

			.inputs = 4,
			.input = {0, },	/* 0 - 3 */
		},

		.element[1] = {
			.type = AUDIO_ELEMENT_PLL,
			.u.pll = {
				.src_sai_id = 5,
				.dst_sai_id = 3,
				.pll_id = kCLOCK_AudioPll1Ctrl,
			},
		},
	},


	.buffers = 4,

	.buffer = {
		[0] = { .flags = AUDIO_BUFFER_FLAG_SHARED_USER, .shared_id = 0},
		[1] = { .flags = AUDIO_BUFFER_FLAG_SHARED_USER, .shared_id = 1},
		[2] = { .flags = AUDIO_BUFFER_FLAG_SHARED_USER, .shared_id = 2},
		[3] = { .flags = AUDIO_BUFFER_FLAG_SHARED_USER, .shared_id = 3},
	},

	.buffer_storage = 0,
};

#endif /* defined(CONFIG_SMP) */

#if (CONFIG_GENAVB_ENABLE == 1)
const struct audio_pipeline_config pipeline_mcr_avb_config = {

	.name = "AVB audio pipeline (with MCR support)",

	.stages = 3,

	.stage[0] = {

		.elements = 3,

		.element[0] = {
			.type = AUDIO_ELEMENT_SINE_SOURCE,
			.u.sine = {
				.freq = 440,
				.amplitude = 0.5,
			},
			.outputs = 1,
			.output = {0},
		},

		.element[1] = {
			.type = AUDIO_ELEMENT_SAI_SOURCE,
			.u.sai_source = {
				.sai_n = 1,
				.sai = {
					[0] = {
						.id = 3,
						.line_n = 1,
						.line = {
							[0] = {
								.channel_n = 2,
							},
						},
					},
				},
			},
			.outputs = 2,
			.output = {1, },	/* 1 - 2 */
		},

		.element[2] = {
			.type = AUDIO_ELEMENT_AVTP_SOURCE,
			.u.avtp_source = {
				.stream_n = 2,
				.stream[0].flags = GENAVB_STREAM_FLAGS_MCR,
				.stream[1].flags = GENAVB_STREAM_FLAGS_MCR,
			},
			.outputs = 4,
			.output = {3, },	/* 3 - 6 */
		},
	},

	.stage[1] = {

		.elements = 1,

		.element[0] = {
			.type = AUDIO_ELEMENT_ROUTING,
			.u.routing = {

			},

			.inputs = 7,
			.input = {0, }, 	/* 0 - 6 */

			.outputs = 6,
			.output = {7, },	/* 7 - 12 */
		},
	},

	.stage[2] = {

		.elements = 2,

		.element[0] = {
			.type = AUDIO_ELEMENT_SAI_SINK,
			.u.sai_sink = {
				.sai_n = 1,
				.sai = {
					[0] = {
						.id = 3,
						.line_n = 1,
						.line = {
							[0] = {
								.channel_n = 2,
							},
						},
					},
				},
			},

			.inputs = 2,
			.input = {7, },	/* 7 - 8 */
		},

		.element[1] = {
			.type = AUDIO_ELEMENT_AVTP_SINK,
			.u.avtp_sink = {
				.stream_n = 2,
			},

			.inputs = 4,
			.input = {9, },	/* 9 - 12 */
		},
	},

	.buffers = 13,

	.buffer_storage = 13,
	.storage = {
		[3] = {.periods = AUDIO_PIPELINE_AVB_MAX_BUFFER_SIZE},
		[4] = {.periods = AUDIO_PIPELINE_AVB_MAX_BUFFER_SIZE},
		[5] = {.periods = AUDIO_PIPELINE_AVB_MAX_BUFFER_SIZE},
		[6] = {.periods = AUDIO_PIPELINE_AVB_MAX_BUFFER_SIZE},
	},
};
#endif /* #if (CONFIG_GENAVB_ENABLE == 1) */

#if defined(CONFIG_SMP) && (CONFIG_GENAVB_ENABLE == 1)

const struct audio_pipeline_config pipeline_mcr_avb_thread_0_config = {

	.name = "AVB audio pipeline for thread 0",

	.stages = 2,

	.stage[0] = {

		.elements = 3,

		.element[0] = {
			.type = AUDIO_ELEMENT_SINE_SOURCE,
			.u.sine = {
				.freq = 440,
				.amplitude = 0.5,
			},
			.outputs = 1,
			.output = {0},
		},

		.element[1] = {
			.type = AUDIO_ELEMENT_SAI_SOURCE,
			.u.sai_source = {
				.sai_n = 1,
				.sai = {
					[0] = {
						.id = 3,
						.line_n = 1,
						.line = {
							[0] = {
								.channel_n = 2,
							},
						},
					},
				},
			},
			.outputs = 2,
			.output = {1, },	/* 1 - 2 */
		},

		.element[2] = {
			.type = AUDIO_ELEMENT_AVTP_SOURCE,
			.u.avtp_source = {
				.stream_n = 2,
				.stream[0].flags = GENAVB_STREAM_FLAGS_MCR,
				.stream[1].flags = GENAVB_STREAM_FLAGS_MCR,
			},
			.outputs = 4,
			.output = {3, },	/* 3 - 6 */
		},
	},

	.stage[1] = {

		.elements = 1,

		.element[0] = {
			.type = AUDIO_ELEMENT_ROUTING,
			.u.routing = {

			},

			.inputs = 7,
			.input = {0, }, 	/* 0 - 6 */

			.outputs = 6,
			.output = {7, },	/* 7 - 12 */
		},
	},

	.buffers = 13,

	.buffer = {
		[7] = {.flags = AUDIO_BUFFER_FLAG_SHARED, .shared_id = 0},
		[8] = {.flags = AUDIO_BUFFER_FLAG_SHARED, .shared_id = 1},
		[9] = {.flags = AUDIO_BUFFER_FLAG_SHARED, .shared_id = 2},
		[10] = {.flags = AUDIO_BUFFER_FLAG_SHARED, .shared_id = 3},
		[11] = {.flags = AUDIO_BUFFER_FLAG_SHARED, .shared_id = 4},
		[12] = {.flags = AUDIO_BUFFER_FLAG_SHARED, .shared_id = 5},
	},
	.buffer_storage = 13,

	.storage = {
		[7] = {.periods = AUDIO_PIPELINE_AVB_MAX_BUFFER_SIZE},
		[8] = {.periods = AUDIO_PIPELINE_AVB_MAX_BUFFER_SIZE},
		[9] = {.periods = AUDIO_PIPELINE_AVB_MAX_BUFFER_SIZE},
		[10] = {.periods = AUDIO_PIPELINE_AVB_MAX_BUFFER_SIZE},
		[11] = {.periods = AUDIO_PIPELINE_AVB_MAX_BUFFER_SIZE},
		[12] = {.periods = AUDIO_PIPELINE_AVB_MAX_BUFFER_SIZE},
	},
};

const struct audio_pipeline_config pipeline_mcr_avb_thread_1_config = {

	.name = "AVB audio pipeline for thread 1",

	.stages = 1,

	.stage[0] = {

		.elements = 2,

		.element[0] = {
			.type = AUDIO_ELEMENT_SAI_SINK,
			.u.sai_sink = {
				.sai_n = 1,
				.sai = {
					[0] = {
						.id = 3,
						.line_n = 1,
						.line = {
							[0] = {
								.channel_n = 2,
							},
						},
					},
				},
			},

			.inputs = 2,
			.input = {0, },	/* 0 - 1 */
		},

		.element[1] = {
			.type = AUDIO_ELEMENT_AVTP_SINK,
			.u.avtp_sink = {
				.stream_n = 2,
			},

			.inputs = 4,
			.input = {2, },	/* 2 - 5 */
		},
	},


	.buffers = 6,

	.buffer = {
		[0] = { .flags = AUDIO_BUFFER_FLAG_SHARED_USER, .shared_id = 0},
		[1] = { .flags = AUDIO_BUFFER_FLAG_SHARED_USER, .shared_id = 1},
		[2] = { .flags = AUDIO_BUFFER_FLAG_SHARED_USER, .shared_id = 2},
		[3] = { .flags = AUDIO_BUFFER_FLAG_SHARED_USER, .shared_id = 3},
		[4] = { .flags = AUDIO_BUFFER_FLAG_SHARED_USER, .shared_id = 4},
		[5] = { .flags = AUDIO_BUFFER_FLAG_SHARED_USER, .shared_id = 5},
	},

	.buffer_storage = 0,
};

#endif /* #if defined(CONFIG_SMP) && (CONFIG_GENAVB_ENABLE == 1) */
