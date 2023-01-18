#!/bin/bash
if  which pdflatex; then
echo "У вас есть Tex и мы попытаемся собрать pdf"
else
read -p 'Установить TEX? Необходим для вывода в PDF (Y/N) ' need_tex
if [ "$need_tex" == "Y" ]; then
  apt-get install -y texlive-latex-extra
fi
fi
echo 'Добро пожаловать, что вы хотите запустить?'
read -p 'Интерпретатор I Генератор G Тесты T ' command
if [ "$command" == "I" ]; then
  echo 'run Interpreter'
  ./build/apps/InterpreterApp/InterpreterApp
fi
if [ "$command" == "G" ]; then
  echo 'run Generator'
  ./build/apps/InputGeneratorApp/InputGeneratorApp
fi
if [ "$command" == "T" ]; then
  echo 'run TestApp'
  ./build/apps/TestsApp/TestsApp
fi
#make
