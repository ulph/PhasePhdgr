import numpy
import pylab

f = open("data.dat", 'rb')
data = numpy.fromfile(f, dtype=numpy.float32)

pylab.plot(data)
pylab.show()
