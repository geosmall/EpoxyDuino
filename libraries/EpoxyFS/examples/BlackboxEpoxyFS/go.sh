#!/bin/bash

make
make run
cd epoxyfsdata
printf "\nrun python3 txt2bbl_v3.py\n"
python3 ../txt2bbl_v3.py
cd ..
echo "go.sh done"