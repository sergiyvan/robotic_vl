import pgrl
from pylab import boxplot, rand, show, figure, ones, concatenate

'''
Zu finden unter:
cd /bin/walkingAnalyser/
python execute_walker11.py

Anweisungen nach der Ausführung befolgen. Es oeffnen sich Xterm Fenster mit "FUmanoid walker". Walkersteuerung mit O (Start des Walkers + Zeitmessung) und P (stop).
Anschließend Distanz ablesen (Maßband) und Zeit aus FUmanoid entnehmen. Eingeben. Weitermachen :)
Sollte ein Sturz passieren, der NICHTS mit den Parametern zu tun hat (Lesefehler, Teppich etc.) immer den Versuch wiederholen ("Retry" durch beantworten der Frage mit NEIN)
Durch nochmaliges O drücken resettet man den Timer, brauchbar wenn bspw. der Walker beim starten Probleme hat! Gemessen wird nur die auf das Maßband projezierte Strecke. (Maximalwert ist 300 ! Keinesfalls mehr. 300 bedeutet auch: "Kein Sturz")

Beachten: IP des Roboters ist in cmdlin.py festgelegt (momentan Grace) und auch der walkertyp steht dort. (walker11)

Probleme:
In /bin/walkingAnalyser/trials/ liegen die Backups mit Uhrzeit und TrialNr.
Die dortigen Zeilen kann man 1:1 in die execute_*.py kopieren (andere Funktionsaufrufe auskommentieren!) und neustarten!
Backups werden nach jeder Iteration erstellt. Man wird aktiv danach gefragt und muss die Anfrage mit JA bestätigen!! Aktuell muss man den setZ_PI() Aufruf auskommentieren. 

'''

# tune FUmanoid walking (param selection by hand - no evidance that it is ideal)
pFUmanoid = ['stepsPerSecond','leglength','stepwidth','stepheight','footroll','torsoLeanFactor']
pValues   = [51.0, 240.0, 31.0, 49.0, 19.0, 0.52] 
pRanges   = [[30.0, 70.0], 
             [200.0, 240.0],
             [27.0, 35.0], 
             [20.0, 50.0], 
             [5.0, 25.0],
             [0.5, 1.5]] 

# transform values
pVTransform = [(pValues[col] - pRanges[col][0]) /  (pRanges[col][1] - pRanges[col][0]) for col in range(len(pRanges))]
pVRestore = [(pVTransform[col] * (pRanges[col][1] - pRanges[col][0])) + pRanges[col][0]  for col in range(len(pRanges))]

# create instance
p = pgrl.pgrl(pFUmanoid, pRanges)

# *** start new PGRL ***
# (#Policies, #IterationenMitvollerStepSize, #MaxIteration, #MaxNumSeqSampling, eta_max, eta_min, InitPolicy)
print p.pgrl (10, 100, 1000, 5, 0.7, 0.3, pVTransform)

# *** BACKUP?
# ADD CODE HERE -- UNCOMMENT lines above !!!!!!!!!!

'''
# Backup 07.06.12
p = pgrl.pgrl( ['stepsPerSecond', 'leglength', 'stepwidth', 'stepheight', 'footroll', 'torsoLeanFactor'] , [[30.0, 70.0], [200.0, 240.0], [27.0, 35.0], [20.0, 50.0], [5.0, 25.0], [0.5, 1.5]] )
#p.setZ_pi(1.148616)
p.pgrl(10, 53, 953, 5, 0.700000, 0.300000, [1, 0.4155532061051582, 0.06223966663142455, 0.21540275862263442, 0.8053530977769237, 0.020000000000000018])
'''
