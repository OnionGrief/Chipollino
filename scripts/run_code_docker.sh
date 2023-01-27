#!/bin/bash
#копируем тест
rm test.txt
cp /original/test.txt ./test.txt
#выполняем программу
./scripts/runcode.sh
#вытаскиваем тест
cp -i ./test.txt /original/test.txt
chmod 777 /original/test.txt
#вытаскиваем дот
cp -i ./resources/report.tex /original/resources/report.tex
chmod 777 /original/resources/report.tex
#вытаскиваем pdf
cp -i ./report.pdf /original/report.pdf  
chmod 777 /original/report.pdf
