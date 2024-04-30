/*
 * Copyright 2023-2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_clock.h"

#include "audio.h"
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

const struct audio_pipeline_config pipeline_dtmf_aud_hat_config = {

	.name = "MX93AUD-HAT DTMF pipeline",

	.stages = 2,

	.stage[0] = {

		.elements = 8,

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
				.sequence_pause_us = 1000000,
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
				.sequence_pause_us = 1000000,
				.amplitude = 0.5,
				.sequence = "#*9876543210DCBA3211",
			},

			.outputs = 1,
			.output = {3},
		},

		.element[4] = {
			.type = AUDIO_ELEMENT_DTMF_SOURCE,
			.u.dtmf = {
				.us = 120000,
				.pause_us = 100000,
				.sequence_pause_us = 1500000,
				.amplitude = 0.5,
				.sequence = "1123ABCD0123456789*#",
			},

			.outputs = 1,
			.output = {4},
		},

		.element[5] = {
			.type = AUDIO_ELEMENT_DTMF_SOURCE,
			.u.dtmf = {
				.us = 120000,
				.pause_us = 100000,
				.sequence_pause_us = 1500000,
				.amplitude = 0.5,
				.sequence = "#*9876543210DCBA3211",
			},
			.outputs = 1,
			.output = {5},
		},

		.element[6] = {
			.type = AUDIO_ELEMENT_DTMF_SOURCE,
			.u.dtmf = {
				.us = 120000,
				.pause_us = 100000,
				.sequence_pause_us = 2000000,
				.amplitude = 0.5,
				.sequence = "1123ABCD0123456789*#",
			},

			.outputs = 1,
			.output = {6},
		},

		.element[7] = {
			.type = AUDIO_ELEMENT_DTMF_SOURCE,
			.u.dtmf = {
				.us = 120000,
				.pause_us = 100000,
				.sequence_pause_us = 2000000,
				.amplitude = 0.5,
				.sequence = "#*9876543210DCBA3211",
			},

			.outputs = 1,
			.output = {7},
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
								.channel_n = 8,
							},
						},
					},
				},
			},

			.inputs = 8,
			.input = {0, }, /* 0 - 7*/
		},
	},

	.buffers = 8,
	.buffer_storage = 8,
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

const struct audio_pipeline_config pipeline_loopback_aud_hat_config = {

	.name = "MX93AUD-HAT Loopback pipeline",

	.stages = 2,

	.stage[0] = {

		.elements = 3,

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
								.channel_n = 6,
							},
						},
					},
				},
			},
			.outputs = 6,
			.output = {0, },	/* 0 - 5 */
		},

		/* Fill last two buffers with sine to prevent unreferenced buffers */
		.element[1] = {
			.type = AUDIO_ELEMENT_SINE_SOURCE,
			.u.sine = {
				.freq = 440, /* la */
				.amplitude = 0.001, /* muted */
			},

			.outputs = 1,
			.output = {6},
		},

		.element[2] = {
			.type = AUDIO_ELEMENT_SINE_SOURCE,
			.u.sine = {
				.freq = 440, /* la */
				.amplitude = 0.001, /* muted */

			},

			.outputs = 1,
			.output = {7},
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
								.channel_n = 8,
							},
						},
					},
				},
			},

			.inputs = 8,
			.input = {0, }, /* 0 - 7 */
		},
	},

	.buffers = 8,
	.buffer_storage = 8,
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

const struct audio_pipeline_config pipeline_sine_aud_hat_config = {

	.name = "MX93AUD-HAT Sinewave pipeline",

	.stages = 2,

	.stage[0] = {

		.elements = 8,

		.element[0] = {
			.type = AUDIO_ELEMENT_SINE_SOURCE,
			.u.sine = {
				.freq = 440, /* la */
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
				.freq = 493, /* si */
				.amplitude = 0.5,
			},

			.outputs = 1,
			.output = {2},
		},

		.element[3] = {
			.type = AUDIO_ELEMENT_SINE_SOURCE,
			.u.sine = {
				.freq = 493,
				.amplitude = 0.5,
			},
			.outputs = 1,
			.output = {3},
		},

		.element[4] = {
			.type = AUDIO_ELEMENT_SINE_SOURCE,
			.u.sine = {
				.freq = 523, /* do */
				.amplitude = 0.5,
			},

			.outputs = 1,
			.output = {4},
		},

		.element[5] = {
			.type = AUDIO_ELEMENT_SINE_SOURCE,
			.u.sine = {
				.freq = 523,
				.amplitude = 0.5,
			},

			.outputs = 1,
			.output = {5},
		},

		.element[6] = {
			.type = AUDIO_ELEMENT_SINE_SOURCE,
			.u.sine = {
				.freq = 587, /* re */
				.amplitude = 0.5,
			},

			.outputs = 1,
			.output = {6},
		},

		.element[7] = {
			.type = AUDIO_ELEMENT_SINE_SOURCE,
			.u.sine = {
				.freq = 587,
				.amplitude = 0.5,
			},

			.outputs = 1,
			.output = {7},
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
								.channel_n = 8,
							},
						},
					},
				},
			},

			.inputs = 8,
			.input = {0, }, /* 0 - 7 */
		},
	},

	.buffers = 8,
	.buffer_storage = 8,
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

const struct audio_pipeline_config pipeline_full_aud_hat_config = {

	.name = "MX93AUD-HAT Full audio pipeline",

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
								.channel_n = 6,
							},
						},
					},
				},
			},
			.outputs = 6,
			.output = {4, },	/* 4 - 9 */
		},
	},

	.stage[1] = {

		.elements = 1,

		.element[0] = {
			.type = AUDIO_ELEMENT_ROUTING,
			.u.routing = {

			},

			.inputs = 10,
			.input = {0, }, 	/* 0 - 9 */

			.outputs = 8,
			.output = {10, },	/* 10 - 17 */
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
								.channel_n = 8,
							},
						},
					},
				},
			},

			.inputs = 8,
			.input = {10, },	/* 10 - 17 */
		},
	},

	.buffers = 18,
	.buffer_storage = 18,
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

const struct audio_pipeline_config pipeline_full_avb_aud_hat_config = {

	.name = "MX93AUD-HAT AVB audio pipeline",

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
								.channel_n = 6,
							},
						},
					},
				},
			},
			.outputs = 6,
			.output = {1, },	/* 1 - 6 */
		},

		.element[2] = {
			.type = AUDIO_ELEMENT_AVTP_SOURCE,
			.u.avtp_source = {
				.stream_n = 2,
			},
			.outputs = 4,
			.output = {7, },	/* 7 - 10 */
		},
	},

	.stage[1] = {

		.elements = 1,

		.element[0] = {
			.type = AUDIO_ELEMENT_ROUTING,
			.u.routing = {

			},

			.inputs = 11,
			.input = {0, }, 	/* 0 - 10 */

			.outputs = 12,
			.output = {11, },	/* 11 - 22 */
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
								.channel_n = 8,
							},
						},
					},
				},
			},

			.inputs = 8,
			.input = {11, },	/* 11 - 18 */
		},

		.element[1] = {
			.type = AUDIO_ELEMENT_AVTP_SINK,
			.u.avtp_sink = {
				.stream_n = 2,
			},

			.inputs = 4,
			.input = {19, },	/* 19 - 22 */
		},
	},

	.buffers = 23,

	.buffer_storage = 23,
	.storage = {
		[7] = {.periods = AUDIO_PIPELINE_AVB_MAX_BUFFER_SIZE},
		[8] = {.periods = AUDIO_PIPELINE_AVB_MAX_BUFFER_SIZE},
		[9] = {.periods = AUDIO_PIPELINE_AVB_MAX_BUFFER_SIZE},
		[10] = {.periods = AUDIO_PIPELINE_AVB_MAX_BUFFER_SIZE},
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

const struct audio_pipeline_config pipeline_mcr_avb_aud_hat_config = {

	.name = "MX93AUD-HAT AVB audio pipeline (with MCR support)",

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
								.channel_n = 8,
							},
						},
					},
				},
			},
			.outputs = 6,
			.output = {1, },	/* 1 - 6 */
		},

		.element[2] = {
			.type = AUDIO_ELEMENT_AVTP_SOURCE,
			.u.avtp_source = {
				.stream_n = 2,
				.stream[0].flags = GENAVB_STREAM_FLAGS_MCR,
				.stream[1].flags = GENAVB_STREAM_FLAGS_MCR,
			},
			.outputs = 4,
			.output = {7, },	/* 7 - 10 */
		},
	},

	.stage[1] = {

		.elements = 1,

		.element[0] = {
			.type = AUDIO_ELEMENT_ROUTING,
			.u.routing = {

			},

			.inputs = 11,
			.input = {0, }, 	/* 0 - 10 */

			.outputs = 12,
			.output = {13, },	/* 13 - 24 */
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
								.channel_n = 8,
							},
						},
					},
				},
			},

			.inputs = 8,
			.input = {13, },	/* 13 - 20 */
		},

		.element[1] = {
			.type = AUDIO_ELEMENT_AVTP_SINK,
			.u.avtp_sink = {
				.stream_n = 2,
			},

			.inputs = 4,
			.input = {21, },	/* 21 - 24 */
		},
	},

	.buffers = 25,

	.buffer_storage = 25,
	.storage = {
		[9] = {.periods = AUDIO_PIPELINE_AVB_MAX_BUFFER_SIZE},
		[10] = {.periods = AUDIO_PIPELINE_AVB_MAX_BUFFER_SIZE},
		[11] = {.periods = AUDIO_PIPELINE_AVB_MAX_BUFFER_SIZE},
		[12] = {.periods = AUDIO_PIPELINE_AVB_MAX_BUFFER_SIZE},
	},
};
#endif /* #if (CONFIG_GENAVB_ENABLE == 1) */

struct play_pipeline_config play_pipeline_dtmf_config = {
	.cfg = {
		&pipeline_dtmf_config,
	}
};

struct play_pipeline_config play_pipeline_dtmf_aud_hat_config = {
	.cfg = {
		&pipeline_dtmf_aud_hat_config,
	}
};

struct play_pipeline_config play_pipeline_loopback_config = {
	.cfg = {
		&pipeline_loopback_config,
	}
};

struct play_pipeline_config play_pipeline_loopback_aud_hat_config = {
	.cfg = {
		&pipeline_loopback_aud_hat_config,
	}
};

struct play_pipeline_config play_pipeline_sine_config = {
	.cfg = {
		&pipeline_sine_config,
	}
};

struct play_pipeline_config play_pipeline_sine_aud_hat_config = {
	.cfg = {
		&pipeline_sine_aud_hat_config,
	}
};

struct play_pipeline_config play_pipeline_full_config = {
	.cfg = {
		&pipeline_full_config,
	}
};

struct play_pipeline_config play_pipeline_full_aud_hat_config = {
	.cfg = {
		&pipeline_full_aud_hat_config,
	}
};

#if defined(CONFIG_SMP)
struct play_pipeline_config play_pipeline_smp_config = {
	.cfg = {
		&pipeline_full_thread_0_config,
		&pipeline_full_thread_1_config,
	}
};
#endif

#if (CONFIG_GENAVB_ENABLE == 1)
struct play_pipeline_config play_pipeline_full_avb_config = {
	.cfg = {
		&pipeline_full_avb_config,
	}
};

struct play_pipeline_config play_pipeline_full_avb_aud_hat_config = {
	.cfg = {
		&pipeline_full_avb_aud_hat_config,
	}
};

struct play_pipeline_config play_pipeline_mcr_avb_config = {
	.cfg = {
		&pipeline_mcr_avb_config,
	}
};

struct play_pipeline_config play_pipeline_mcr_avb_aud_hat_config = {
	.cfg = {
		&pipeline_mcr_avb_aud_hat_config,
	}
};
#endif

#if defined(CONFIG_SMP) && (CONFIG_GENAVB_ENABLE == 1)
struct play_pipeline_config play_pipeline_avb_smp_config = {
	.cfg = {
		&pipeline_full_avb_thread_0_config,
		&pipeline_full_avb_thread_1_config,
	}
};

struct play_pipeline_config play_pipeline_mcr_smp_config = {
	.cfg = {
		&pipeline_mcr_avb_thread_0_config,
		&pipeline_mcr_avb_thread_1_config,
	}
};
#endif

const struct play_pipeline_config *g_play_config[] = {
	[0] = &play_pipeline_dtmf_config,
	[1] = &play_pipeline_sine_config,
	[2] = &play_pipeline_loopback_config,
	[3] = &play_pipeline_full_config,
#if (CONFIG_GENAVB_ENABLE == 1)
	[4] = &play_pipeline_full_avb_config,
#endif
#if defined(CONFIG_SMP)
	[5]  = &play_pipeline_smp_config,
#endif
#if (CONFIG_GENAVB_ENABLE == 1)
	[6] = &play_pipeline_mcr_avb_config,
#endif
#if defined(CONFIG_SMP) && (CONFIG_GENAVB_ENABLE == 1)
	[7] = &play_pipeline_avb_smp_config,
	[8] = &play_pipeline_mcr_smp_config,
#endif
};

const struct play_pipeline_config *g_play_aud_hat_config[] = {
	[0] = &play_pipeline_dtmf_aud_hat_config,
	[1] = &play_pipeline_sine_aud_hat_config,
	[2] = &play_pipeline_loopback_aud_hat_config,
	[3] = &play_pipeline_full_aud_hat_config,
#if (CONFIG_GENAVB_ENABLE == 1)
	[4] = &play_pipeline_full_avb_aud_hat_config,
#endif
#if defined(CONFIG_SMP)
	[5]  = &play_pipeline_smp_config,
#endif
#if (CONFIG_GENAVB_ENABLE == 1)
	[6] = &play_pipeline_mcr_avb_aud_hat_config,
#endif
#if defined(CONFIG_SMP) && (CONFIG_GENAVB_ENABLE == 1)
	[7] = &play_pipeline_avb_smp_config,
	[8] = &play_pipeline_mcr_smp_config,
#endif
 };
