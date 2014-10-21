#!/bin/bash

# use first argument as branch name, or master by default
BRANCH=${1:-master}

# if the berlinunited repository is not yet defined, add it
git remote | grep berlinunited || git remote add berlinunited git@fumanoids.de:berlinunited

# do the actually pulling and subtree merge
git pull -s subtree berlinunited $BRANCH

######################################################################################################################
# initial setup was done according to http://www.kernel.org/pub/software/scm/git/docs/howto/using-merge-subtree.html
# though step 3 was skipped as we previously extracted the berlinunited folder from the FUmanoids project as described
# in http://stackoverflow.com/questions/359424/detach-subdirectory-into-separate-git-repository
#
# git remote add -f berlinunited git@fumanoids.de:berlinunited
# git merge -s ours --no-commit berlinunited/master
# git read-tree --prefix=berlinunited/ -u berlinunited/master
