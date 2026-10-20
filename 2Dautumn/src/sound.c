#include "headerfile/SOUND.h"
#include "headerfile/GAME.h"

typedef struct {
	float freqStart;
	float freqEnd;
	float duration;
	float volume;
	float noise;  // 0 = pure tone, 1 = pure noise
	bool square;
}t_SfxShape;

static const t_SfxShape sfxShapes[SFX_COUNT] = {
	/* SFX_HIT       */ { 320, 120, 0.10f, 0.35f, 0.5f, true },
	/* SFX_HARVEST   */ { 700, 500, 0.06f, 0.25f, 0.3f, true },
	/* SFX_EMBER     */ { 900, 200, 0.22f, 0.25f, 0.7f, false },
	/* SFX_HURT      */ { 200, 70, 0.25f, 0.40f, 0.3f, true },
	/* SFX_ENEMY_DIE */ { 500, 80, 0.30f, 0.30f, 0.8f, false },
	/* SFX_CRAFT     */ { 520, 1040, 0.15f, 0.25f, 0.0f, true },
	/* SFX_FEED      */ { 300, 600, 0.30f, 0.25f, 0.4f, false },
	/* SFX_NIGHT     */ { 110, 55, 1.20f, 0.35f, 0.1f, false },
	/* SFX_DAWN      */ { 440, 880, 0.80f, 0.30f, 0.0f, false },
};

static Mix_Chunk* chunks[SFX_COUNT];
static Uint8* buffers[SFX_COUNT];
static bool soundReady = false;

static Mix_Chunk* synthesize(const t_SfxShape* shape, int freq, int channels, Uint8** bufferOut)
{
	int frames = (int)(shape->duration * freq);
	Uint32 bytes = (Uint32)(frames * channels * sizeof(Sint16));
	Sint16* samples = (Sint16*)SDL_malloc(bytes);
	float phase = 0.0f;

	if (samples == NULL)
		return NULL;
	for (int i = 0; i < frames; i++)
	{
		float t = (float)i / (float)frames;
		float f = shape->freqStart + (shape->freqEnd - shape->freqStart) * t;
		phase += f / (float)freq;
		phase -= SDL_floorf(phase);

		float tone = shape->square ? (phase < 0.5f ? 1.0f : -1.0f) : SDL_sinf(phase * 6.2831f);
		float noise = RandomFloat(-1.0f, 1.0f);
		float envelope = (1.0f - t) * (1.0f - t);
		float attack = i < 64 ? i / 64.0f : 1.0f;
		float value = (tone * (1.0f - shape->noise) + noise * shape->noise) * envelope * attack * shape->volume;

		for (int c = 0; c < channels; c++)
			samples[i * channels + c] = (Sint16)(value * 32000.0f);
	}
	*bufferOut = (Uint8*)samples;
	return Mix_QuickLoad_RAW((Uint8*)samples, bytes);
}

void InitSound(void)
{
	int freq = 0, channels = 0;
	Uint16 format = 0;

	// no audio device or an unexpected format: the game simply stays silent
	if (Mix_QuerySpec(&freq, &format, &channels) == 0 || format != AUDIO_S16SYS)
		return;
	for (int i = 0; i < SFX_COUNT; i++)
		chunks[i] = synthesize(&sfxShapes[i], freq, channels, &buffers[i]);
	Mix_AllocateChannels(16);
	soundReady = true;
}

void PlaySfx(SFX sfx)
{
	if (!soundReady || chunks[sfx] == NULL)
		return;
	Mix_PlayChannel(-1, chunks[sfx], 0);
}

void FreeSound(void)
{
	if (!soundReady)
		return;
	Mix_HaltChannel(-1);
	for (int i = 0; i < SFX_COUNT; i++)
	{
		if (chunks[i] != NULL)
			Mix_FreeChunk(chunks[i]);
		SDL_free(buffers[i]);
		chunks[i] = NULL;
		buffers[i] = NULL;
	}
	soundReady = false;
}
