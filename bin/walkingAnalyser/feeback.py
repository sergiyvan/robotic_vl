import random

N = 1000
disNoise  = 0.3
timeNoise = 0.5

class feedback :

    def __init__(self):
        self.c = 0 

    def measureSimTimeDis(self):
        
        b = 0 if self.c < 600 else (self.c - 600) / 400.0 
        dis = random.gauss(3.0 + b, disNoise)
        dis = dis if dis <= 3.0 else 3.0
        
        timeTmp = 25 - (self.c / 75.0)
        time = random.gauss(timeTmp, timeNoise)

        self.c +=1
    
        return dis, time
    
    
#for i in range(N):
#    f = feedback()
#    d, t = f.measureSimTimeDis()
#    print 'dis:',d, 'time:',t
