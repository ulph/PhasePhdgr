import numpy
import pylab

f = open("data.dat", 'rb')
data = numpy.fromfile(f, dtype=numpy.float32)

t = numpy.arange(start=0, stop=len(data)) / 48000.

pylab.plot(t, data)
pylab.show()
