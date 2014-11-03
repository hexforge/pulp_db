import sys
import random

#09:00:00.000
#18:59:59.999

hours = 9
mins = 0
seconds = 0
mill = 0

while (True):
        mill += 1
        if mill == 1000:
            seconds += 1
            mill = 0
        if seconds == 60:
            mins += 1
            seconds = 0
        if mins == 60:
            hours += 1
            mins = 0

        if hours == 18:
            break
        print("{0:0>2}:{1:0>2}:{2:0>2}.{3:0>3}".format(hours, mins, seconds, mill))
