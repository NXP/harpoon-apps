/*
 * Copyright 2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "audio_pipeline.h"

struct audio_pipeline_config pipeline_config = {
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
				.sai_n = 3,
				.sai = {
					[0] = {
						.id = 0,
						.line_n = 1,
						.line = {
							[0] = {
								.channel_n = 2,
							},
						},
					},
					[1] = {
						.id = 5,
						.line_n = 1,
						.line = {
							[0] = {
								.channel_n = 2,
							},
						},
					},
					[2] = {
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

			.outputs = 6,
			.output = {10, },	/* 10 - 15 */
		},
	},

	.stage[2] = {

		.elements = 1,

		.element[0] = {
			.type = AUDIO_ELEMENT_SAI_SINK,
			.u.sai_sink = {
				.sai_n = 3,
				.sai = {
					[0] = {
						.id = 0,
						.line_n = 1,
						.line = {
							[0] = {
								.channel_n = 2,
							},
						},
					},
					[1] = {
						.id = 5,
						.line_n = 1,
						.line = {
							[0] = {
								.channel_n = 2,
							},
						},
					},
					[2] = {
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

			.inputs = 6,
			.input = {10, },	/* 10 - 15 */
		},
	},

	.buffers = 16,

	.buffer_storage = 16,
};
