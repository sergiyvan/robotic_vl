import feedback
import random
from math import *
from cmdlin import setupRobotControlSW, query_yes_no

N = 100  # measurements
dev = 2  # pitch and roll
IMUData = [[random.gauss(0.0, 5.0) for row in range(N)] for col in range(dev)]
trialDuration = 20.0 # seconds

    
 
# --------------------------------------------------------------------------- #       

class fitness :

    def __init__(self, policynames, policyranges):
        self.f = feedback.feedback() 
        self.pNames = policynames
        self.pRange = policyranges

    # central function to get fitness values
    # Def. Fitness:  Moeglichst weit Laufen in 20s (trialDuration)
    #                   + wenig Schwanken (Pitch + Roll) 
    #                   + gute Odometrie abschaetzung
    def getFitnessValue(self,policy) :

        #print '-------------------------------------------------'
        #print 'New test trial. Duration for a trial is', trialDuration, 'sec.'
        #print 'Params: ', policy
        
        policy = [(policy[col] * (self.pRange[col][1] - self.pRange[col][0])) + self.pRange[col][0]  for col in range(len(self.pRange))]
        
        b = True
        while b :
            setupRobotControlSW(self.pNames, policy)
            p = ''.join(["[%s: %.3f]" % (self.pNames[col],policy[col]) for col in range(len(policy))]) 
            print 'Current Policy:',p
            b = not query_yes_no("Could you test a trial with the current policy (No = Retry!)")
        
        # User feedback
        #disReal, t = self.f.measureSimTimeDis() # deactivate generated fitness
        (disReal, t) = self.promptFeedback()
        v = (disReal/100.0) / float(t)
        
        # calc fitness
        #print 'Distance:', disReal, 'cm  /  Velocity:', v
        f = self.fitness_nimbro(disReal, v)
        
        print 'Fitness of last trial: %.4f achieving %.1f cm  vel: %.3f m/s.\n' % (f, disReal, v)
        
        return f

# --------------------------------------------------------------------------- #

    # We can evaluate a trial without human help
    # So we ask the user for some data...
    def promptFeedback(self) :
        
        b = True
        disR  = 0
        timeR = 0
        while b :
            disR = raw_input('Gib die erreichte Distanz ein (cm): ')
            timeR = raw_input('Gib die benoetigte Zeit ein (s): ')
            try:
                dis = float( disR )
                time = float( timeR )
            except:
                print( "Invalid number (dis is an int. time is float!!)" )
                continue # type in correct number!
            
            b = not query_yes_no("Input correct? (No => Do it again)")
        
        return (dis, time)

# --------------------------------------------------------------------------- #    

    def fitness_nimbro(self,distance, velocity) :
        d_exp = 300.0 # in cm
        v_max = 0.666 # m/s

        #print 'fitness (',distance, ',',velocity,')'

        if distance < d_exp :
            #print (velocity / v_max) * (distance / d_exp)
            return (velocity / v_max) * (distance / d_exp)
        else :
            # belohne stabilitaet
            #print (velocity / v_max) + 1   
            return (velocity / v_max) + 1   
