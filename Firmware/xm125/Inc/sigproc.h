// Copyright (c) Acconeer AB, 2024
// All rights reserved

#ifndef SIGPROC_H_
#define SIGPROC_H_

#include <stdbool.h>
#include <stdint.h>



/**
 * @brief Struct representing a high-pass filter
 */
typedef struct
{
	float alpha;
	float squeeze;
	float prev_y;
	float prev_x;
	bool  has_prev_x;
} high_pass_filter_t;

/**
 * @brief Struct representing a real-time peak detector
 */
typedef struct
{
	uint16_t buffer_size;
	uint16_t min_distance;
	float    min_height;
	uint16_t n_pos_samples;
	float   *buffer;
	uint16_t index;
	uint16_t samples_since_last_peak;
} real_time_peak_finder_t;

/**
 * @brief Initialize a high-pass filter
 *
 * @param[out] filter Pointer to the filter struct
 * @param[in] alpha Smoothing factor (e.g., 0.995)
 * @param[in] squeeze Suppression threshold (e.g., 1.5)
 */
void high_pass_filter_init(high_pass_filter_t *filter, float alpha, float squeeze);

/**
 * @brief Update high-pass filter with new sample
 *
 * @param[in,out] filter Pointer to filter
 * @param[in] x New input value
 * @return Filtered output
 */
float high_pass_filter_update(high_pass_filter_t *filter, float x);

/**
 * @brief Initialize a real-time peak finder
 *
 * @param[out] pf Pointer to the peak finder struct
 * @param[in] buffer_size Sliding window size (e.g., 3)
 * @param[in] min_height Minimum height for peak detection (e.g., 0.2)
 * @param[in] min_distance Minimum spacing between peaks (e.g., 5)
 * @param[in] n_pos_samples Number of positive samples after peak (e.g., 4)
 * @param[in] buffer Memory for internal buffer of size buffer_size
 */
void peak_finder_init(real_time_peak_finder_t *pf, uint16_t buffer_size, float min_height, uint16_t min_distance, uint16_t n_pos_samples, float *buffer);

/**
 * @brief Add a sample and check for peak
 *
 * @param[in,out] pf Pointer to the peak finder
 * @param[in] value New sample value
 * @return True if currently within a peak response window
 */
bool peak_finder_add_sample(real_time_peak_finder_t *pf, float value);

#endif
