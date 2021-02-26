#rm -rf Experiments/*
#QT="insertQueries10000.txt"
QT="queriesI0Shuffled.txt"
mkdir -p Experiments/$QT
#cmake -G "Unix Makefiles" .
cmake -S . -B build
cmake --build build -j --clean-first
cmake --install build
./KDBTree $PWD $QT
#python visualizeHopTree.py
