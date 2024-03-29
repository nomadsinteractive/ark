from typing import Optional

from ark import ByteArray, FloatArray


class Generator:
    NOISE_TYPE_SIMPLEX = 0
    NOISE_TYPE_PERLIN = 0

    def __init__(self, noise_type: int = NOISE_TYPE_SIMPLEX, seed: int = 0, frequence: float = 1.0):
        pass

    @property
    def seed(self) -> int:
        return 0

    @seed.setter
    def seed(self, seed: int):
        pass

    @property
    def frequence(self) -> float:
        return 0

    @frequence.setter
    def frequence(self, frequence: float):
        pass

    def set_fractal_octaves(self, octaves: int):
        pass

    def set_fractal_gain(self, gain: float):
        pass

    def set_fractal_lacunarity(self, lacunarity: float):
        pass

    def set_fractal_weighted_strength(self, weighted_strength: float):
        pass

    def noise_map2d(self, bounds: tuple[int, int, int, int]) -> FloatArray:
        pass
