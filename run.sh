meson compile -C build
Dataset=AIS
ln -sf ../DATA/${Dataset}/Queries .
ln -sf ../DATA/${Dataset}/fullData.txt dataFile.txt
# Q=(Rng)
# T=(S)
# W=(W)
# for q in ${Q[@]}; do
#     for t in ${T[@]}; do
#         for w in ${W[@]}; do
#             QF="${q}/${t}-L1e7-I1e6-${w}H"
#             echo "Experiments/${QF}"
#             mkdir -p "Experiments/${QF}"
#             ./Index $PWD $QF 204 204
#             # lldb -- Index $PWD $QF 204 204
#         done
#     done
# done
mkdir -p "Experiments/Bulk/Q8e3"
./Index $PWD "Bulk/Q8e3" 204 204
# lldb -- Index $PWD "Bulk/Q8e3" 204 204
