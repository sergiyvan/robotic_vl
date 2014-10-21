''' 

Policy Gradient Reinforcement Learning

Based on: Stochastic Optimization of Bipedal Walking using
            Gyro Feedback and Phase Resetting 
              (Felix Faber and Sven Behnke)
              
HowTo:
- Declare options/ params to optimise
- Use init values that work (!) - We are optimizing not learning in general :)
- Select/Define fitness function
- Start script PGRL
- Each fitness fct call opens an ssh connection to a robot
- Provide human feedback
- Do 1000 fitness function calls!
- Done.

'''

import random
import datetime
import fitness 
from matrix import *
from math import *
from cmdlin import query_yes_no
from pylab import boxplot, plot, rand, show, figure, ones, concatenate

#
def lengthVec (vector) :
    length = 0.0
    for i in range(len(vector)) :
        length += vector[i]**2
        
    length = sqrt(length)
    return length

#
def normalize(vector) :
    length = lengthVec(vector)
     
    for i in range(len(vector)) :
        vector[i] = vector[i] / length
        
    return vector
    
def normListSumTo(L, sumTo=1):
    '''normalize values of a list to make it sum = sumTo'''

    sum = reduce(lambda x,y:x+y, L)
    return [ x/(sum*1.0)*sumTo for x in L]
    
def limitByRange(boundries, value):
    value = value if value >= boundries[0] else boundries[0]
    value = value if value <= boundries[1] else boundries[1]
    
    return value
        
# ------------------------------------------------------------------------------------------------------      
# -----------------PGRL----------------------------------------------

class pgrl:

    def __init__(self,policynames, policyranges):
        self.policynames  = policynames
        self.policyranges = policyranges
        self.iterCount    = 0
        self.f = fitness.fitness(policynames, policyranges) 
        self.stat_avg_z         = [[]]
        self.stat_avg_z_total   = []
        
    # B policies will be generated
    # g steps until eta_max decreases
    # SM maximum of fitness calls
    # M  max. fitness calls seqSampling
    # eta min/max
    # x_init initial paramset
    def pgrl (self,B, g, SM_init, M, eta_max, eta_min, x_init) :
        
        params  = len(x_init)
        eps     = 0.015
        eta     = eta_max
        SM      = SM_init

        x_pi     = x_init
        f_pi     = 0.0
        policies = []
        
        s_names = ['-','0','+']
        
        # average fitness of current policy
        z_pi     = 0;
        
        print '---------------------------START PGRL-----------------------------------'
        print '*** Calc. Z_PI by three fitness fct calls'
        # calc avg policy fitness of x_pi
        for i in range(3) :
            z_pi += self.f.getFitnessValue(x_init)
        z_pi /= 3.0  
        
        print "*** Initial Z_PI: %.5f" % z_pi
        self.stat_avg_z_total.append(z_pi)
		#best_policy = (z_pi, x_init)

        sm = 0
        
        # improve x_pi step by step 
        # until eta < eta_min
        while eta_min < eta :  
        
            print ''
            print '***************************************************************'
            print 'New Iteration (eta:', eta, ' eta_min:',eta_min,')' 
            
            # The algorithm randomly generates B test policies 
            # {x1 , x2 , . . . , xB } around policy x_pi
            policies = self.randGenPolicies(x_pi, B, eps)
            #policies.append(best_policy[1]) # always carry old best settings in new generation
                 
            # 3 Buckets for each dimension
            # S-, S0, S+ pro j
            # adress: S[j][0..2]
            S = [[[],[],[]] for col in range(len(x_pi))]
            
            
            print '---------------SS--------------'
            # Sequential Sampling 
            # Evaluate policies with good fitness more often 
            # for better results. Weak policies are thrown away quickly
            
            for i in range(len(policies)) :
                # single fitness function evalution
                ## f = fitness(policies[i], it) 
                
                print '\n* SeqSampling for Policy[',i,']' 
                
                # Sequential Sampling!
                v  =  0.2 # 20% diff is enough evidence: threshold fitness difference
                z, m = self.seqSampling(z_pi, policies[i], v, M)
                
                print 'Average fitness after seqSampling: %.4f (%d trails evaluated)' % (z, m)

				#if z > best_policy[0] :
			  	#	best_policy = (z, policies[i])              
		
                # add value to stats
                self.storeStats(z)
                # add to sm storage
                sm += m
                
                
                for j in range(len(x_pi)) :
                    if x_pi[j] - policies[i][j] < 0 :
                        S[j][2].append(z) # S+
                    elif  x_pi[j] - policies[i][j] > 0 :
                        S[j][0].append(z) # S-
                    else :
                        S[j][1].append(z) # S0
            
            #sm_list.append(sm)
                        
            # [PRINT] buckets after evaluating the policies
            #for i in range(len(x_pi)) :
            #    print 'x_i[',i,']:'
            #    for j in range(3) :
            #        print 'S[',s_names[j],']:', S[i][j] 
                        
                        
            # Average of each fitness for each group of param j
            avgGroupFitness = [[0.0, 0.0, 0.0] for j in range(len(x_pi))]
            
            for j in range(len(x_pi)) :
                for k in range(3) :
                    avgGroupFitness[j][k] = sum(S[j][k]) / len(S[j][k]) if len(S[j][k]) != 0.0 else 0.0
                    
                    
            # [PRINT] buckets after evaluating the policies
            #for i in range(len(x_pi)) :
            #    print 'x_i[',i,']:'
            #    for j in range(3) :
            #        print 'avgGF[',s_names[j],']:', avgGroupFitness[i][j] 
            
                
            # Adjustment vector a
            a = [0.0 for col in range(len(x_pi))]
            
            # calc adjustment vector entries
            for j in range(len(x_pi)) :
                if avgGroupFitness[j][1] > avgGroupFitness[j][2] \
                    and avgGroupFitness[j][1] > avgGroupFitness[j][0] :
                   a[j] = 0
                   z_pi += avgGroupFitness[j][1]
                else :
                   a[j] = avgGroupFitness[j][2] - avgGroupFitness[j][0]
                   if avgGroupFitness[j][0] >= avgGroupFitness[j][2] :
                        z_pi += avgGroupFitness[j][0]
                   else :
                        z_pi += avgGroupFitness[j][2]
            
            # update avg fitnis of policy pi (z_pi)
            z_pi /= len(x_pi)
            self.stat_avg_z_total.append(z_pi)
        
            # normalize when adjustment vector is not filled with 0.0
            checksum = sum(map(abs,a))
            if checksum > 0.0 :
                a = normalize(a)
                print 'Adjustmentvector:', a, ' Length:',lengthVec(a)
                print 'Old x_pi:',x_pi 
                print 'Z_PI:', z_pi
            else :
                print 'ERROR: This should not happen in the middle of a session.'

            # conversion to matrix class data types         
            Mx_pi = matrix([x_pi])
            Meta  = matrix([[eta]])
            Ma    = matrix([a])
            
            # update x_pi
            Mx_pi = Mx_pi + Meta * Ma
            #Mx_pi.show()
            x_pi = [ limitByRange([0,1], Mx_pi.value[0][col]) for col in range(len(Mx_pi.value[0])) ]
              
            print 'Iteration complete. New x_pi (upcoming generation):', x_pi
            
            # Adaptive step size
            # sm  number of fitness fct calls
            # SM  max number of fitness fct calls
            old_eta = eta
                 
            if sm < g :
                #adaptive step size
                eta = eta_max
            else :
                eta = eta_max - (((sm - g) * (eta_max - eta_min)) / (SM - g))
            
                
            # save learning state
            if query_yes_no("Do you want to backup the current PGRL step?") :
                self.saveToFile(sm, (B, g - sm if g - sm > 0 else 0.0, SM_init - sm, M, eta, eta_min, x_pi), z_pi)
                self.plotStats(False) # TODO
            if not query_yes_no("Do you want to go on?") :
                break
            
            # update counter
            self.iterCount += 1

            # stats          
            print 'Iteration #',self.iterCount,' is finished. New eta:',eta,' (old:',old_eta,') #fitness fct calls:',sm
        
        # final stat
        print 'Finished:  It took ',sm,'trials (max.',SM,') and',self.iterCount,'iterations in total.'
        
        # save result
        self.saveToFile(sm, (B, g - sm if g - sm > 0 else 0.0, SM_init - sm, M, eta, eta_min, x_pi), z_pi)
        
        return x_pi

# ------------------------------------------------------------------------------------------------------      

    # generate B policies from parent x_parent
    # Each dimension can be changed by eps
    def randGenPolicies (self,x_parent, B, eps) :
        
        policies = []
        tmp = 0
        
        for i in range(B) :
            x_new = []
            for j in range(len(x_parent)) :
                rand = (random.random() * 3)
                if rand < 1 :
                    tmp = x_parent[j]
                elif rand >= 1 and rand < 2:
                    tmp = x_parent[j] - eps
                else :
                    tmp = x_parent[j] + eps
                
		tmp = limitByRange([0,1], tmp)    
                x_new.append(tmp)
            
            policies.append(x_new)
                
        return policies

# ------------------------------------------------------------------------------------------------------      

    # Input: Average policy fitness z_pi , test policy xi , threshold
    #       for fitness difference v, max. no. of samplings M

    # Output: Average fitness z_i of policy xi , number of samplings used m


    def seqSampling(self,z_pi, policy, v, M) :
        z = 0.0
        
        threshold = z_pi * v
        # average fitness
        z_avg = self.f.getFitnessValue(policy)
        d     = abs(z_avg  - z_pi)
        
        # Try for max. M times to evaluate policy i
        for m in range(2, M) :
            if abs(d) > threshold : 
                # if d is larger then our threshold, return average fitness
                # idea: If the fitnesses of the two policies are very different, 
                #       only few samples suffice for estimating the direction 
                #       of the gradient of the fitness function
                print 'END seqSampling> abs(d: %.1f) > threshold: %.2f (threshold) [trial avg: %.4f (pgrl avg: %.4f) trials: %d]' % (d,threshold,z_avg,z_pi,m-1)
                return z_avg, m-1
            else :
                # run another trail, get average and update d
                z     = self.f.getFitnessValue(policy)
                z_avg = (z_avg * m + z) / (m + 1)
                d     = abs(z_avg - z_pi)
        
        # return average fitness + number of trials
        print 'END seqSampling> abs(d: %.1f) <= threshold: %.2f trial avg: %.4f (pgrl avg: %.4f) trials: %d' % (d,threshold,z_avg,z_pi,M)
        return z_avg, M

# ------------------------------------------------------------------------------------------------------      

    # store each avg_z value of a policy in every iteration
    def storeStats(self, avg_z) :
        iter_entries = len(self.stat_avg_z)
        
        # if no entry for iter exists
        if self.iterCount > iter_entries - 1 :
            self.stat_avg_z.append([])
            
        # store a pair of id and value
        self.stat_avg_z[self.iterCount].append(avg_z)
        
# ------------------------------------------------------------------------------------------------------      

    def plotStats(self,save) :
        
        figure()
        # show boxplot, iff we have enough data
        if min(map(len, self.stat_avg_z)) > 3 :        
            data = self.stat_avg_z
            boxplot(data,1)
        #else :
        figure()
        data2 = self.stat_avg_z_total
        plot(data2)
        show()

# ------------------------------------------------------------------------------------------------------      

    def saveToFile(self, i, fct_params, z_pi) :

        now = datetime.datetime.now()
        
        filename = 'trials/%d-%d-%d--%d-%d-backup_trial_%d.txt' % (now.year, now.month, now.day, now.hour, now.minute, i)
        f = open(filename,'w')
        
        # set information
        introduction = '# This file was automatically generated after %d trials of a PGRL session\n# in order to optimise the following params:' % i
        setZ_pi      = 'p.setZ_pi(%f)' % z_pi
        start_call   = 'p.pgrl(%d, %d, %d, %d, %f, %f, %s)' % fct_params
        
        
        print >>f, introduction, self.policynames
        print >>f, 'p = pgrl.pgrl(',self.policynames,',', self.policyranges ,')'
        print >>f, setZ_pi
        print >>f, start_call
        #print >>f, 'pRanges:', self.policyranges
        
        print 'Backup > Your data was saved into the file (',filename,')'

# ------------------------------------------------------------------------------------------------------      
