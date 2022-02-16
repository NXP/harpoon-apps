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
				.amplitude = 1.0,
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
				.amplitude = 1.0,
				.sequence = "#*9876543210DCBA3211",
			},

			.outputs = 1,
			.output = {1},
		},

		.element[2] = {
			.type = AUDIO_ELEMENT_SINE_SOURCE,
			.u.sine = {
				.freq = 440,
				.amplitude = 1.0,
			},
			.outputs = 1,
			.output = {2},
		},

		.element[3] = {
			.type = AUDIO_ELEMENT_SINE_SOURCE,
			.u.sine = {
				.freq = 880,
				.amplitude = 1.0,
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
				},
			},
			.outputs = 4,
			.output = {4, 5, 6, 7},
		},
	},

	.stage[1] = {

		.elements = 1,

		.element[0] = {
			.type = AUDIO_ELEMENT_ROUTING,
			.u.routing = {

			},

			.inputs = 8,
			.input = {0, 1, 2, 3, 4, 5, 6, 7},

			.outputs = 4,
			.output = {8, 9, 10, 11},
		},
	},

	.stage[2] = {

		.elements = 1,

		.element[0] = {
			.type = AUDIO_ELEMENT_SAI_SINK,
			.u.sai_sink = {
				.sai_n = 2,
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
				},
			},

			.inputs = 4,
			.input = {8, 9, 10, 11},
		},
	},

	.buffers = 12,

	.buffer[0] = {
		.storage = 0,
	},

	.buffer[1] = {
		.storage = 1,
	},

	.buffer[2] = {
		.storage = 2,
	},

	.buffer[3] = {
		.storage = 3,
	},

	.buffer[4] = {
		.storage = 4,
	},

	.buffer[5] = {
		.storage = 5,
	},

	.buffer[6] = {
		.storage = 6,
	},

	.buffer[7] = {
		.storage = 7,
	},

	.buffer[8] = {
		.storage = 8,
	},

	.buffer[9] = {
		.storage = 9,
	},

	.buffer[10] = {
		.storage = 10,
	},

	.buffer[11] = {
		.storage = 11,
	},

	.buffer_storage = 12,

	.storage[0] = {
		.periods = 2,
	},

	.storage[1] = {
		.periods = 2,
	},

	.storage[2] = {
		.periods = 2,
	},

	.storage[3] = {
		.periods = 2,
		//.base = &my_memory, /* pre-allocated storage */
	},

	.storage[4] = {
		.periods = 2,
	},

	.storage[5] = {
		.periods = 2,
	},

	.storage[6] = {
		.periods = 2,
	},

	.storage[7] = {
		.periods = 2,
	},

	.storage[8] = {
		.periods = 2,
	},

	.storage[9] = {
		.periods = 2,
	},

	.storage[10] = {
		.periods = 2,
	},

	.storage[11] = {
		.periods = 2,
	},
};
