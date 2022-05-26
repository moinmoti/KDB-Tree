meson compile -C build
Q=(Rng)
T=(S)
W=(W)
for q in ${Q[@]}; do
    for t in ${T[@]}; do
        for w in ${W[@]}; do
            QF="${q}/${t}-L1e7-I1e6-${w}H"
            echo "Experiments/${QF}"
            mkdir -p "Experiments/${QF}"
            ./Index $PWD $QF 204 204
            # lldb -- Index $PWD $QF 204 204
        done
    done
done
# mkdir -p "Experiments/Bulk/L7e7-Q8e3"
# ./Index $PWD "Bulk/L7e7-Q8e3" 204 204
# lldb -- Index $PWD "Bulk/L7e7-Q8e3" 204 204
