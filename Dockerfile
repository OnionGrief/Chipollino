FROM ubuntu:18.04 as chipollino
ENV TZ=Europe/Moscow
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone
RUN apt-get update
#COPY . /app/

COPY . ./app
WORKDIR /app/
RUN apt-get install -y make cmake dos2unix wget dot2tex build-essential
RUN ./scripts/install_packages.sh 
#все делаем скриптом (скрипт не только в докере запускать можно)

CMD ["./scripts/run_code_docker.sh"]
