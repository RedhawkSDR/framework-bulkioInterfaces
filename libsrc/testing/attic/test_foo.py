
import sys
from test_cntl import *


nchan=1
wname = "Bwave"
print "ARGS(" + str(len(sys.argv)) + ")" + str(sys.argv)

#
# attach do domain first,  this affects sys.argv default REDHAWK_DEV
#
attach('REDHAWK_DEV')

if len(sys.argv) > 1 :
    nchan=int(sys.argv[1])

if len(sys.argv) > 2 :
    wname=sys.argv[2]

def track_datain():
    track_stats('TestJava', 'dataShortIn' )



start_waveforms(wname, nchan)


