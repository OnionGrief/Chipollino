#!/bin/bash
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
