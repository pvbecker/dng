import rawpy
import matplotlib.pyplot as plt

with rawpy.imread("output.dng") as raw:
    data = raw.raw_image.copy()
    print("type of data: ", type(data[0][0]))
    print("data: ", data[0:200][0:200])

