#!/bin/bash
if [ "$EUID" -ne 0 ]; then
    echo "Скрипт должен быть запущен с использованием sudo."
    exit 1
fi
BASEDIR=$(pwd)
for test_file in $BASEDIR/scripts/integrationTests/*; do
    sudo rm -rf *report* *pdflatex*
    echo "$test_file"
    sudo $BASEDIR/build/apps/InterpreterApp/InterpreterApp $test_file
    # Проверка наличия и непустоты файла rendered_report.pdf
    if [ ! -s "rendered_report.pdf" ]; then
        echo "Ошибка: Файл rendered_report.pdf отсутствует или пуст."
        exit 1
    fi

    # Проверка наличия и непустоты файла report.pdf
    if [ ! -s "report.pdf" ]; then
        echo "Ошибка: Файл report.pdf отсутствует или пуст."
        exit 1
    fi
done