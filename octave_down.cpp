#include <Arduino.h>
#include "octave_down.h"
#include "utility/dspinst.h"

static bool first = true;

void AudioEffectOctaveDown::init()
{
	memset(blocklist, 0, sizeof(blocklist));
	window_length = 4600;
	window_offset = 1200;
	update_window();
	bypass = true;
	two_octaves = false;
}

void AudioEffectOctaveDown::update_window()
{
	float phase_inc = 3.14159f / (float)window_length;
	float scale = 56000.0f * (float)window_offset / (float)window_length;
	for (int i=0; i < window_length; i++) {
		// TODO: make the window shape configurable
		window[i] = sinf((float)i * phase_inc) * scale;
	}
	num_window_index = (window_length + window_offset - 1) / window_offset;
	for (int i=0; i < num_window_index; i++) { 
		window_index[i] = i * window_offset * -1;
	}
}

static void stretch2x(int16_t *dest, const int16_t *src, const int16_t *window, uint32_t len)
{
	while (len >= 2) {
		int16_t s = *src++;
		*dest++ += (s * *window++) >> 15;
		*dest++ += (s * *window++) >> 15;
		len -= 2;
	}
}

void AudioEffectOctaveDown::stretch2x_b(int16_t *dest, int offset, const int16_t *win, uint32_t len)
{
	if (offset >= 0) {
		stretch2x(dest, blocklist[0]->data + offset, win, len);
	} else {
		offset *= -1;
		int bnum = (offset - 1) / 128 + 1;
		int remain = (128 - (offset % 128)) % 128; // is there a cleaner way?
		int available = (128 - remain) * 2;
		if (len <= available) { 
			stretch2x(dest, blocklist[bnum]->data + remain, win, len);
		} else {
			stretch2x(dest, blocklist[bnum]->data + remain, win, available);
			stretch2x(dest + available, blocklist[bnum-1]->data,
				win + available, len - available);
		}
	}
}

void AudioEffectOctaveDown::update(void)
{
	audio_block_t *block;

	block = receiveReadOnly(0);
	if (!block) return;
	if (bypass) {
		transmit(block);
		release(block);
		return;
	}
	if (blocklist[OCTAVEDOWN_BLOCK_LIST_SIZE-1])
		release(blocklist[OCTAVEDOWN_BLOCK_LIST_SIZE-1]);
	for (int i=OCTAVEDOWN_BLOCK_LIST_SIZE-1; i > 0; i--) {
		blocklist[i] = blocklist[i-1];
	}
	blocklist[0] = block;
	block = allocate();
	if (!block) return;
	memset(block->data, 0, sizeof(block->data));

	for (int i=0; i < num_window_index; i++) {
		int index = window_index[i];
		if (index > window_length - 128) {
			int remain = window_length - index;
			stretch2x_b(block->data, -index / 2, window + index, remain);
			index -= window_offset * num_window_index;
		} else if (index >= 0) { 
			stretch2x_b(block->data, -index / 2, window + index, 128);
		} else if (index > -128) {
			int offset = index * -1;
			stretch2x_b(block->data + offset, offset, window, 128 - offset);
		}
		window_index[i] = index + 128;
	}

	transmit(block);
	release(block);
	first = false;
}
