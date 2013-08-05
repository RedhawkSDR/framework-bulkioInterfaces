import sys
from ossie.utils import sb

dsource=sb.DataSource()
dsink=sb.DataSink()
test_comp=sb.Component(sys.argv[1])
data=range(100)
dsource.connect(test_comp, providesPortName='dataShortIn')
test_comp.connect(dsink, providesPortName='shortIn', usesPortName='dataShortOut')
sb.start()
dsource.push(data, EOS=True)
dest_data=dsink.getData(eos_block=True)
