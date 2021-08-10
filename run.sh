#rm -rf Experiments/*
QF="qI0"
#QF="knnTestQueries.txt"
mkdir -p Experiments/$QF
#cmake -G "Unix Makefiles" .
cmake -S . -B build
cmake --build build -j --clean-first
cmake --install build
# lldb -- KDBTree $PWD $QF $1 $2
# ./KDBTree $PWD $QF $1 $2
params=(250 500 750 1000)
for p in "${params[@]}"; do
    ./KDBTree $PWD $QF 300 "$p"
done
#python visualizeHopTree.py
