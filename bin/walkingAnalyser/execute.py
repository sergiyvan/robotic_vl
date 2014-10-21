import pgrl
from pylab import boxplot, rand, show, figure, ones, concatenate


# tune FUmanoid walking (param selection by hand - no evidance that it is ideal)
pFUmanoid = ['leglength','stepwidth','footroll','zGain','zPhase', 'xPhase', 'ShoulderPhase']
pValues   = [230.0, 10.0, 5.0, 5.0, 20.0, 0.0, 0.0] # stepwidth too large -> was reduced
pRanges   = [[220.0, 240.0],
             [2.0, 20.0], 
             [5.0, 25.0],
	     [1.0, 10.0],
             [10.0, 50.0],
             [0.0, .49],
             [0.0, .49]]

pVTransform = [(pValues[col] - pRanges[col][0]) /  (pRanges[col][1] - pRanges[col][0]) for col in range(len(pRanges))]
pVRestore = [(pVTransform[col] * (pRanges[col][1] - pRanges[col][0])) + pRanges[col][0]  for col in range(len(pRanges))]

# TODO good values
#policy = [0.7872547136598661, 1, 0.87461863061403544, 0.51688078807384241, 0.79223036762489263, 0.0]
#policy = [(policy[col] * (pRanges[col][1] - pRanges[col][0])) + pRanges[col][0]  for col in range(len(pRanges))]
#print policy


#p = pgrl.pgrl(pFUmanoid, pRanges)

# (B, g, S, eta_max, eta_min, x_init)
#print p.pgrl (10, 100, 1000, 5, 0.7, 0.3, pVTransform) # 10, 100, 0.7, 0.3
#print p.pgrl (10, 0, 450, 5, 0.7, 0.3, pVTransform) 
#print p.pgrl (10, 0, 450, 1, 0.7, 0.3, pVTransform) 


p = pgrl.pgrl( ['leglength', 'stepwidth', 'footroll', 'zGain', 'zPhase', 'xPhase', 'ShoulderPhase'] , [[220.0, 240.0], [2.0, 20.0], [5.0, 25.0], [1.0, 10.0], [10.0, 50.0], [0.0, 0.499], [0.0, 1.0]] )
#p.setZ_pi(0.053768)
p.pgrl(10, 0, 700, 5, 0.611111, 0.300000, [0, 0.8964247683294648, 0.2570946296390511, 0.7293498567745491, 0.577359626636868, 0.3211059154292084, 0.32068168799240215])

# -------------------------------------------------------------------------------
'''
spread= rand(50) * 100
center = ones(25) * 50
flier_high = rand(10) * 100 + 100
flier_low = rand(10) * -100
data =concatenate((spread, center, flier_high, flier_low), 0)

# fake up some more data
spread= rand(50) * 100
center = ones(25) * 40
flier_high = rand(10) * 100 + 100
flier_low = rand(10) * -100
d2 = concatenate( (spread, center, flier_high, flier_low), 0 )
data.shape = (-1, 1)
d2.shape = (-1, 1)
#data = concatenate( (data, d2), 1 )
# Making a 2-D array only works if all the columns are the
# same length.  If they are not, then use a list instead.
# This is actually more efficient because boxplot converts
# a 2-D array into a list of vectors internally anyway.
data = [data, d2, d2[::1,0], concatenate((rand(50) * 75, rand(5) * -125), 0)]

print rand(50) * 75
#print d2[::2,0]

figure()
# notched boxplot:
# Die Box entspricht dem Bereich, in dem die mittleren 50 % der Daten liegen. 
# Sie wird also durch das obere und das untere Quartil begrenzt und die Laenge der Box 
# entspricht dem Interquartilsabstand (englisch interquartile range, IQR).
# Des Weiteren wird der Median als durchgehender Strich in der Box eingezeichnet. 
# Dieser Strich teilt das gesamte Diagramm in zwei Haelften, in denen jeweils 50 % der Daten liegen
# Einkerbungen symbolisieren die Konfidenzintervalle fuer den Median.
# Der aeussere Bereich um die Box beschreibt den Bereich ohne Ausreisser - alle Datenpunkte ausserhalb werden als
# Ausreisser klassifiziert
boxplot(data,1)

show()
'''
