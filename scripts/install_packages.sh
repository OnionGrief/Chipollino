#!/bin/bash
BASEDIR=$(pwd)
apt-get install -y make cmake dos2unix wget dot2tex build-essential
read -p 'Установить TEX? Необходим для вывода в PDF (Y/N) ' need_tex
if [ "$need_tex" == "Y" ]; then
  apt-get install -y texlive-latex-extra
fi
# install refal5
if grep "refal" ~/.bashrc; then
echo "У вас уже есть РЕФАЛ, вы прекрасны!"
else
echo "Ставлю РЕФАЛ"
cd /usr/src
mkdir refal && cd refal
wget http://www.botik.ru/pub/local/scp/refal5/ref5_081222.zip
unzip ref5_081222.zip 
rm makefile
cp makefile.lin makefile
echo "export PATH=$PATH:/usr/src/refal" >> ~/.bashrc
fi
cd $BASEDIR
rm -fR ./build
mkdir build
cd ./build/
cmake ../. && cmake --build .
cat ./../scripts/chipollino.txt
#Надо написать сборку рефал-программ когда они будут в одной ветке, возможно еще что-то надо (для текущего комиита все собирается)
#make
