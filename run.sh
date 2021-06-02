#rm -rf Experiments/*
QF="queriesI0Shuffled.txt"
#QF="knnTestQueries.txt"
mkdir -p Experiments/$QF
#cmake -G "Unix Makefiles" .
cmake -S . -B build
cmake --build build -j --clean-first
cmake --install build
./KDBTree $PWD $QF $1 $2
#python visualizeHopTree.py
