#!/bin/bash
make
for q in 500000; do
    for file in dna200; do
        for m in 12 16 32 64; do
            for type in std dbl lut2 lut2-dbl; do
                sudo sh -c "sync; echo 3 > /proc/sys/vm/drop_caches"
                taskset -c 0 ./test/countSA $type $file $q $m
            done
            for type in std-hash std-hash-dense dbl-hash dbl-hash-dense; do
                for lf in 0.9; do
                    sudo sh -c "sync; echo 3 > /proc/sys/vm/drop_caches"
                    taskset -c 0 ./test/countSA $type 12 $lf $file $q $m
                done
            done
        done
    done
done
for q in 500000; do
    for file in proteins200; do
        for m in 5 16 32 64; do
            for type in std dbl lut2 lut2-dbl; do
                sudo sh -c "sync; echo 3 > /proc/sys/vm/drop_caches"
                taskset -c 0 ./test/countSA $type $file $q $m
            done
            for type in std-hash std-hash-dense dbl-hash dbl-hash-dense; do
                for lf in 0.9; do
                    sudo sh -c "sync; echo 3 > /proc/sys/vm/drop_caches"
                    taskset -c 0 ./test/countSA $type 5 $lf $file $q $m
                done
            done
        done
    done
done
for q in 500000; do
    for file in english200 sources200 xml200; do
        for m in 8 16 32 64; do
            for type in std dbl lut2 lut2-dbl; do
                sudo sh -c "sync; echo 3 > /proc/sys/vm/drop_caches"
                taskset -c 0 ./test/countSA $type $file $q $m
            done
            for type in std-hash std-hash-dense dbl-hash dbl-hash-dense; do
                for lf in 0.9; do
                    sudo sh -c "sync; echo 3 > /proc/sys/vm/drop_caches"
                    taskset -c 0 ./test/countSA $type 8 $lf $file $q $m
                done
            done
        done
    done
done