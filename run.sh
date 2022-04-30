#cmake -G "Unix Makefiles" .
cmake -S . -B build
cmake --build build -j --clean-first
# cmake --install build
# Q=(Knn Rng)
# T=(T)
# W=(W E R)
# for q in ${Q[@]}; do
#     for t in ${T[@]}; do
#         for w in ${W[@]}; do
#             QF="${q}/${t}-L1e7-I1e6-${w}H"
#             echo "Experiments/${QF}"
#             mkdir -p "Experiments/${QF}"
#             ./KDBTree $PWD $QF 128 128
#             # lldb -- Index $PWD $QF 128 128
#         done
#     done
# done
mkdir -p "Experiments/Bulk/L7e7-Q8e3"
./KDBTree $PWD "Bulk/L7e7-Q8e3" 128 128
# lldb -- Index $PWD $QF $1 $2
