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

			.outputs = 4,
			.output = {9, },	/* 9 - 12 */
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
			.input = {9, },	/* 9 - 12 */
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

	.buffers = 13,

	.buffer_storage = 13,
};
#endif /* #if (CONFIG_GENAVB_ENABLE == 1) */
