#include "AudioStream.h"

#define OCTAVEDOWN_BLOCK_LIST_SIZE 48

class AudioEffectOctaveDown : public AudioStream
{
public:
	AudioEffectOctaveDown(void): AudioStream(1,inputQueueArray) { init(); }
	virtual void update(void);
	void begin(float window_ms, float offset_ms) {
		window_length = window_ms * (AUDIO_SAMPLE_RATE_EXACT * 0.001) + 4.5;
		window_offset = offset_ms * (AUDIO_SAMPLE_RATE_EXACT * 0.001) + 4.5;
		window_length &= ~7;
		window_offset &= ~7;
		if (window_length > 12000) window_length = 12000;
		if (window_length < 600) window_length = 600;
		if (window_offset < window_length / 20) window_offset = (window_length / 20) & ~7;
		if (window_offset > window_length / 2) window_offset = (window_length / 2) & ~7;
		bypass = false;
	}
	void begin(void) { bypass = false; }
	void end(void) { bypass = true; }
private:
	void init();
	void update_window();
	void stretch2x_b(int16_t *dest, int offset, const int16_t *win, uint32_t len);
	void stretch4x_b(int16_t *dest, int offset, const int16_t *win, uint32_t len);
	audio_block_t *inputQueueArray[1];
	audio_block_t *blocklist[OCTAVEDOWN_BLOCK_LIST_SIZE];
	int window_offset; // should be a multiple of 8
	int window_length; // should be a multiple of 8
	int num_window_index;
	int window_index[24];
	int16_t window[12000];
	bool two_octaves;
	bool bypass;
};
