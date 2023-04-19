#!/bin/bash
BASEDIR=$(pwd)
apt-get update
apt-get install -y make cmake dos2unix wget dot2tex build-essential dos2unix
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
make
echo "export PATH=$PATH:/usr/src/refal" >> ~/.bashrc
fi
cd $BASEDIR
rm -fR ./build
mkdir build
cd ./build/
cmake ../. && cmake --build .
cd ../refal 
dos2unix MathMode.ref
dos2unix Postprocess.ref
dos2unix Preprocess.ref
dos2unix TestGenerator.ref
dos2unix FrameFormatter.ref
dos2unix RunFormatter.ref
export PATH=$PATH:/usr/src/refal
refc MathMode.ref
refc Postprocess.ref
refc Preprocess.ref
refc TestGenerator.ref
refc FrameFormatter.ref
refc RunFormatter.ref
cat ./../scripts/chipollino.txt
#Надо написать сборку рефал-программ когда они будут в одной ветке, возможно еще что-то надо (для текущего комиита все собирается) - вроде есть на будущее
#make
