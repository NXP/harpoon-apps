/*
 * Copyright 2023-2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_clock.h"
#include "audio_pipeline.h"
#if (CONFIG_GENAVB_ENABLE == 1)
#include "genavb/control_clock_domain.h"
#endif

const struct audio_pipeline_config pipeline_dtmf_config = {

	.name = "DTMF pipeline",

	.stages = 2,

	.stage[0] = {

		.elements = 2,

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

	},

	.stage[1] = {

		.elements = 1,

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
			.input = {0, 1},
		},
	},

	.buffers = 2,

	.buffer_storage = 2,
};


const struct audio_pipeline_config pipeline_loopback_config = {

	.name = "Loopback pipeline",

	.stages = 2,

	.stage[0] = {

		.elements = 1,

		.element[0] = {
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
			.output = {0, },	/* 0 - 1 */
		},
	},

	.stage[1] = {

		.elements = 1,

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
	},

	.buffers = 2,

	.buffer_storage = 2,
};


const struct audio_pipeline_config pipeline_sine_config = {

	.name = "Sinewave pipeline",

	.stages = 2,

	.stage[0] = {

		.elements = 2,

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
	},

	.stage[1] = {

		.elements = 1,

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
			.input = {0, 1},
		},
	},

	.buffers = 2,

	.buffer_storage = 2,
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
			.output = {4, },	/* 4 - 5 */
		},
	},

	.stage[1] = {

		.elements = 1,

		.element[0] = {
			.type = AUDIO_ELEMENT_ROUTING,
			.u.routing = {

			},

			.inputs = 6,
			.input = {0, }, 	/* 0 - 5 */

			.outputs = 2,
			.output = {6, },	/* 6 - 7 */
		},
	},

	.stage[2] = {

		.elements = 1,

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
			.input = {6, },	/* 6 - 7 */
		},
	},

	.buffers = 8,

	.buffer_storage = 8,
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
				.clock_domain = GENAVB_CLOCK_DOMAIN_DEFAULT,
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
				.clock_domain = GENAVB_CLOCK_DOMAIN_DEFAULT,
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

#ifdef CONFIG_SMP
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
			.output = {4, },	/* 4 - 5 */
		},
	},

	.stage[1] = {

		.elements = 1,

		.element[0] = {
			.type = AUDIO_ELEMENT_ROUTING,
			.u.routing = {

			},

			.inputs = 6,
			.input = {0, }, 	/* 0 - 5 */

			.outputs = 2,
			.output = {6, },	/* 6 - 7 */
		},
	},

	.buffers = 8,

	.buffer = {
		[6] = {.flags = AUDIO_BUFFER_FLAG_SHARED, .shared_id = 0},
		[7] = {.flags = AUDIO_BUFFER_FLAG_SHARED, .shared_id = 1},
	},
	.buffer_storage = 8,

	.storage = {
		[6] = {.periods = 3},
		[7] = {.periods = 3},
	},
};

const struct audio_pipeline_config pipeline_full_thread_1_config = {

	.name = "Full audio pipeline for thread 1",

	.stages = 1,

	.stage[0] = {

		.elements = 1,

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
	},

	.buffers = 2,

	.buffer = {
		[0] = { .flags = AUDIO_BUFFER_FLAG_SHARED_USER, .shared_id = 0},
		[1] = { .flags = AUDIO_BUFFER_FLAG_SHARED_USER, .shared_id = 1},
	},

	.buffer_storage = 0,
};
#endif

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
				.clock_domain = GENAVB_CLOCK_DOMAIN_0,
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
				.clock_domain = GENAVB_CLOCK_DOMAIN_0,
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
