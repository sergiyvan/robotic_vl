#!/bin/bash -e

# Install the dependencies for the desktopbuild of FUmanoid

# determine linux distro
if [[ -e /etc/fedora-release ]]; then
  distro=fedora
elif [ $(which lsb_release 2>/dev/null) ]; then
  distro=$(lsb_release -d | awk {'print $2'} | tr '[:upper:]' '[:lower:]')
  if [ $distro == "linux" ]; then
    distro=$(lsb_release -d | awk {'print $3'} | tr '[:upper:]' '[:lower:]')
  fi
else
  # looking for release files - ugly - should work
  # http://linuxmafia.com/faq/Admin/release-files.html
  distro=$(ls /etc/ | egrep '[-_](release|version)$' | awk -F [-_] {'print $1'} \
           | tr '[:upper:]' '[:lower:]')
fi

case $distro in
  ubuntu|mint)

    # Tested on Ubuntu 10.10
    # Tested on Ubuntu 11.04
    # Tested on Ubuntu 11.10
    # Tested on Ubuntu 12.04
    # Tested on Ubuntu 14.04
    # Tested on Linux Mint Lisa

    packages="\
ack-grep \
astyle \
build-essential \
ccache \
default-jdk \
default-jre \
distcc \
distcc-pump \
doxygen \
git \
glade \
graphviz \
joe \
libblas-dev \
libboost-all-dev \
libespeak-dev \
libfile-find-rule-perl \
libglademm-2.4-dev \
libgtest-dev \
libgtkmm-2.4-dev \
liblapack-dev \
liblist-moreutils-perl \
libncurses5-dev \
libopencv-dev \
libprotobuf-dev \
libsdl1.2-dev \
libterm-readkey-perl \
libusb-1.0-0-dev \
libusb-dev \
python-matplotlib \
python-numpy \
python-opencv \
python-protobuf \
python-pydot \
python-pygame \
python-vte \
rsync \
ruby \
subversion \
valgrind \
xsltproc \
"

    uname -a | grep x86_64 >/dev/null && packages="$packages \
lib32z1 lib32gcc1 lib32stdc++6"

    echo "Installing all dependencies: $packages"

    sudo apt-get install $packages

    ;;

  arch)

    # Tested on Arch Linux May 2011
    
    echo
    echo "[WARNING] On x86_84 systems make sure to uncomment multilib in /etc/pacman.conf"
    echo

    packages="glade opencv libusb python-pygame python-matplotlib \
 python-numpy libxslt graphviz libglademm gtkmm protobuf doxygen vte espeak rsync \
 joe distcc ccache rsync boost"

    uname -a | grep x86_64 >/dev/null && packages="$packages \
lib32-zlib lib32-gcc-libs lib32-libstdc++5"

    echo "Installing all dependencies: $packages"

    sudo pacman -S $packages

    ;;

  fedora)
    
    packages="\
astyle \
ccache \
gcc \
gcc-c++ \
java
distcc \
doxygen \
git \
glade \
graphviz \
joe \
blas \
blas-devel \
opencv \
opencv-devel \
espeak \
espeak-devel \
lapack \
lapack-devel \
protobuf-devel \
perl-TermReadKey \
python-matplotlib \
numpy \
opencv-python \
python-pip \
protobuf-python \
pygame \
vte \
rsync \
ruby \
subversion \
valgrind \
boost-devel \
"

    uname -a | grep x86_64 >/dev/null && packages="$packages \
libstdc++.i686 glibc-devel.i686 zlib.i686"

    echo "Installing all dependencies: $packages"

    sudo yum install $packages

    ;;

  *)

    echo "Sorry, your OS is not supported yet"

    ;;
esac

