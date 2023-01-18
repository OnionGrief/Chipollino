#!/bin/bash
cd ..
sudo docker run -it --rm --name runchipollino -v $(pwd):/original/ chipollino

#sudo docker build -t chipollino .