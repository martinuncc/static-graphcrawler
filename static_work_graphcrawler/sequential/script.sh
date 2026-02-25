#!/bin/bash
#SBATCH --job-name=static-graphcrawler
#SBATCH --partition=Centaurus
#SBATCH --time=00:20:00
#SBATCH --nodes=1
#SBATCH --ntasks-per-node=1
#SBATCH --mem=10G

make
./level_client "Tom Hanks" 1
./par_level_client "Tom Hanks" 1

./level_client "Tom Hanks" 2
./par_level_client "Tom Hanks" 2

./level_client "Tom Hanks" 3
./par_level_client "Tom Hanks" 3

