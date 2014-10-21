import sys
import os

# from stackoverflow
# http://stackoverflow.com/questions/3041986/python-command-line-yes-no-input

def query_yes_no(question, default="yes"):
    """Ask a yes/no question via raw_input() and return their answer.

    "question" is a string that is presented to the user.
    "default" is the presumed answer if the user just hits <Enter>.
        It must be "yes" (the default), "no" or None (meaning
        an answer is required of the user).

    The "answer" return value is one of "yes" or "no".
    """
    valid = {"yes":True,   "y":True,  "ye":True,
             "no":False,     "n":False}
    if default == None:
        prompt = " [y/n] "
    elif default == "yes":
        prompt = " [Y/n] "
    elif default == "no":
        prompt = " [y/N] "
    else:
        raise ValueError("invalid default answer: '%s'" % default)

    while True:
        sys.stdout.write(question + prompt)
        choice = raw_input().lower()
        if default is not None and choice == '':
            return valid[default]
        elif choice in valid:
            return valid[choice]
        else:
            sys.stdout.write("Please respond with 'yes' or 'no' "\
                             "(or 'y' or 'n').\n")

# set params with current policy
def setupRobotControlSW (paramnames, x_pi) :
    # ssh -C -t -Y root@192.168.0.21 'killall FUmanoid; ./FUmanoid walker'
    
    # 21 Konrad
    # 22 Grace
    
    if len(paramnames) == len(x_pi) :
        cmd_params = ''.join([" --motions.walker11.%s %.5f" % (paramnames[col],x_pi[col]) for col in range(len(x_pi))]) 
        #command = "xterm -e ../../build/pc/FUmanoid%s walker" % cmd_params
        command = "xterm -e ssh -C -t -Y root@192.168.0.22 'killall FUmanoid; ./FUmanoid --motions.locomotion walker11%s walker'" % cmd_params

        #print command,'&'
        os.system(''.join([command,' &'])) 
 
# TEST 
#setupRobotControlSW(["a","b","c"],[1,2,3])
