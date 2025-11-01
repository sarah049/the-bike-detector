class HighPassFilter:
    def __init__(self, ALPHA, SQUEEZE):
        self.ALPHA = ALPHA
        self.SQUEEZE = SQUEEZE
        self.prev_y = 0
        self.prev_x = None

    def update(self, x):
        if self.prev_x is None:
            self.prev_x = x
            return 0
        y = self.ALPHA * (self.prev_y + x - self.prev_x) if x > self.SQUEEZE else 0
        self.prev_y = y
        self.prev_x = x
        return y