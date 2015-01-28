#!/bin/bash

alias m=make

cd ./plugins;

cd ./context;
cd ./curl  && make install && cd -;
cd ./db4   && make install && cd -;
cd ./pccts && make install && cd -;
cd ./xml   && make install && cd -;
cd ./redland && make install && cd -;

cd ../eagenerators/
cd ./xfsnative  && make install && cd -;
cd ./magick  && make install && cd -;
cd ./png  && make install && cd -;
cd ./jpg  && make install && cd -;
cd ./mpeg2  && make install && cd -;
cd ./a52  && make install && cd -;
cd ./redland && make install && cd -;
