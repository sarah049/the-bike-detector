#include "sigproc.h"

void high_pass_filter_init(high_pass_filter_t *filter, float alpha, float squeeze)
{
	filter->alpha      = alpha;
	filter->squeeze    = squeeze;
	filter->prev_y     = 0.0f;
	filter->prev_x     = 0.0f;
	filter->has_prev_x = false;
}

float high_pass_filter_update(high_pass_filter_t *filter, float x)
{
	if (!filter->has_prev_x)
	{
		filter->prev_x     = x;
		filter->has_prev_x = true;
		return 0.0f;
	}

	float y = (x > filter->squeeze) ? filter->alpha * (filter->prev_y + x - filter->prev_x) : 0.0f;

	filter->prev_y = y;
	filter->prev_x = x;

	return y;
}

void peak_finder_init(real_time_peak_finder_t *pf,
                      uint16_t                 buffer_size,
                      float                    min_height,
                      uint16_t                 min_distance,
                      uint16_t                 n_pos_samples,
                      float                   *buffer)
{
	pf->buffer_size             = buffer_size;
	pf->min_height              = min_height;
	pf->min_distance            = min_distance;
	pf->n_pos_samples           = n_pos_samples;
	pf->buffer                  = buffer;
	pf->index                   = 0U;
	pf->samples_since_last_peak = min_distance;

	for (uint16_t i = 0U; i < buffer_size; i++)
	{
		buffer[i] = 0.0f;
	}
}

static bool peak_finder_is_peak(real_time_peak_finder_t *pf)
{
	uint16_t center = pf->buffer_size / 2U;
	float    value  = pf->buffer[center];

	if (value < pf->min_height)
	{
		return false;
	}

	for (uint16_t i = 0U; i < pf->buffer_size; i++)
	{
		if (i == center)
		{
			continue;
		}

		if (pf->buffer[i] >= value)
		{
			return false;
		}
	}

	return true;
}

bool peak_finder_add_sample(real_time_peak_finder_t *pf, float value)
{
	// Shift left and insert new value
	for (uint16_t i = 0U; i < pf->buffer_size - 1U; i++)
	{
		pf->buffer[i] = pf->buffer[i + 1U];
	}

	pf->buffer[pf->buffer_size - 1U] = value;
	pf->samples_since_last_peak++;

	if (peak_finder_is_peak(pf))
	{
		if (pf->samples_since_last_peak >= pf->min_distance)
		{
			pf->samples_since_last_peak = 0U;
		}
	}

	return (pf->samples_since_last_peak < pf->n_pos_samples);
}
