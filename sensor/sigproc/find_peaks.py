import numpy as np

class RealTimePeakFinder:
    """
    A simple real-time peak finder that detects peaks based on a sliding window.
    Delay is BUF_SIZE // 2 samples
    """
    def __init__(self, buffer_size=11, min_height=2, min_distance=5, n_pos_samples=4):
        self.buffer_size = buffer_size
        self.buffer = [0] * buffer_size
        self.min_distance = min_distance
        self.min_height = min_height
        self.samples_since_last_peak = min_distance  # Start "ready"
        self.n_pos_samples = n_pos_samples

    def _is_peak(self, window):
        center = len(window) // 2
        value = window[center]

        if value < self.min_height:
            return False

        for i in range(len(window)):
            if i == center:
                continue
            if window[i] >= value:
                return False

        return True

    def add_sample(self, value):
        self.buffer.pop(0)
        self.buffer.append(value)

        self.samples_since_last_peak += 1

        if self._is_peak(self.buffer):
            if self.samples_since_last_peak >= self.min_distance:
                self.samples_since_last_peak = 0

        if self.samples_since_last_peak < self.n_pos_samples:
            return True
        else:
            return False
        

def find_peaks(signal, buffer_size=11, min_height=2, min_distance=5, n_pos_samples=4):
    peak_finder = RealTimePeakFinder(buffer_size, min_height, min_distance, n_pos_samples)
    peak_finder_out = [peak_finder.add_sample(s) for s in signal]

    # remove BUF_SIZE // 2 from start
    for _ in range(buffer_size // 2):
        peak_finder_out.pop(0)

    peaks = np.array([i for i in range(len(peak_finder_out)) if peak_finder_out[i]])    # peaks are all peaks indecies
    return peaks