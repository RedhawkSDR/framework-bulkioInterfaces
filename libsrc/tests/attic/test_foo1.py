import sys
from ossie.utils import sb

dsource=sb.DataSource()
dsink=sb.DataSink()
test_comp=sb.Component('Foo')
data=range(100)
dsource.connect(test_comp, providesPortName='dataShortIn')
test_comp.connect(dsink, providesPortName='shortIn', usesPortName='dataShortOut')
sb.start()
dsource.push(data)
dest_data=dsink.getData()
