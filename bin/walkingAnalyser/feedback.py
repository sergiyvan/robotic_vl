import random

N = 1000
disConst = 300
disNoise  = 10
timeNoise = 0.5

class feedback :

    def __init__(self):
        self.c = 0 

    def measureSimTimeDis(self):
        
        if self.c < 1000 :
            b = 0 if self.c < 600 else (self.c - 600) / 400.0 
            dis = random.gauss(disConst + b, disNoise)
            dis = dis if dis <= disConst else disConst
            
            timeTmp = 25 - (self.c / 75.0)
            time = random.gauss(timeTmp, timeNoise)
        else :
            dis  = disConst
            time = random.gauss(11, 0.1)
            
        self.c +=1
        
        return dis, time
    
    
#for i in range(N):
#    f = feedback()
#    d, t = f.measureSimTimeDis()
#    print 'dis:',d, 'time:',t
