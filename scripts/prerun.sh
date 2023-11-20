#!/bin/bash
if [ "$EUID" -ne 0 ]; then
    echo "Скрипт должен быть запущен с использованием sudo."
    exit 1
fi
with_tex=false
BASEDIR=$(pwd)
while [[ $# -gt 0 ]]; do
  case "$1" in
    --with-tex)
      with_tex=true
      shift
      ;;
    *)
      echo "Неверный аргумент: $1"
      exit 1
      ;;
  esac
done

# Добавление ключа репозитория
curl -s --compressed "https://xtoter.github.io/apt_packages/KEY.gpg" | gpg --dearmor | sudo tee /etc/apt/trusted.gpg.d/apt_packages.gpg >/dev/null
# Добавление репозитория
sudo curl -s --compressed -o /etc/apt/sources.list.d/my_list_file.list "https://xtoter.github.io/apt_packages/my_list_file.list"
# Обновление списка пакетов
apt update
apt install -y make cmake dos2unix wget dot2tex build-essential dos2unix refal5-old
if [ "$with_tex" = true ];  then
  apt install -y texlive-latex-extra texlive-fonts-extra texlive-science texlive-lang-cyrillic

fi
# Начало компиляции ------------------------
echo "Начало сборки"
# Сборка рефал частей
cd $BASEDIR/refal 
# Чистим все собранное
rm -f *rsl

for file in *.ref; do
    # Проверка, существует ли файл
    if [ -e "$file" ]; then
        # Конвертация формата из DOS в UNIX
        echo "Файл $file"
        dos2unix "$file"
        refc "$file"
    fi
done

# Сборка чиполлино
cd $BASEDIR
rm -fR ./build
mkdir build
cd ./build/
cmake ../. && cmake --build .
# Конец
cat ./../scripts/chipollino.txt