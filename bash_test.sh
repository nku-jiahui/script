#!/usr/bin/env bash
REPO_DIR=$(cd $(dirname $BASH_SOURCE[0])/.. && pwd)
# realloc是变量宽度与寄存器
REALLOC_RESULT_PATH=${1:-"$REPO_DIR/data/reallocResult.txt"}
ATOM_TRANSLATE=$REPO_DIR/tmp/build/bin/atom-translate
ATOM_OPT=$REPO_DIR/tmp/build/bin/atom-opt

# 变量映射表路径
BASE_NAME_MAPPING_TABLE_PATH=${4:-"$REPO_DIR/data/reflection.txt"}

# 变量映射表路径
BASE_NAME_MAPPING_TABLE_PATH=${4:-"$REPO_DIR/data/reflection.txt"}

BASH_FILE_PATH=${2:-"$REPO_DIR/data/test_data"}
BASH_FILE_OUTPUT_PATH=${3:-"$REPO_DIR/tmp/output/${BASH_FILE_PATH##*/}"}
# BASH_FILE_PATH是bash文件夹的路径,格式为:
# bash_test/
# ├── single_test1/
# │   ├── hard_acc
# │   │   └── soft_fuc1.txt
# │   │   └── soft_fuc2.txt
# │   │   └── soft_fuc3.txt
# │   │   └── ....
# │   └── hard_acc.txt
# ├── single_test2/
# │   ├── hard_acc
# │   │   └── soft_fuc1.txt
# │   │   └── soft_fuc2.txt
# │   │   └── soft_fuc3.txt
# │   │   └── ....
# │   └── hard_acc.txt
# └── single_test3/
# .   ├── hard_acc
# .   │   └── soft_fuc1.txt
# .   │   └── soft_fuc2.txt
# .   │   └── soft_fuc3.txt
# .   │   └── ....
# .   └── hard_acc.txt
# .
total_test_num=0
total_success_test_num=0
total_fail_test_num=0
total_fail_compare_test_num=0
total_bad_test_num=0
total_num=0
acc_success_test_num=0
acc_fail_test_num=0
acc_fail_ending_test_num=0

LOG_FILE=$BASH_FILE_OUTPUT_PATH/bash_test_result.txt
# 清空LOG_FILE
> $LOG_FILE

for file in $BASH_FILE_PATH/*; do
# 确认是文件夹
    if [ ! -d "$file" ]; then
        continue
    fi
    single_test_num=0
    single_success_test_num=0
    single_fail_test_num=0
    single_fail_compare_test_num=0
    single_fail_ending_test_num=0
    single_bad_test_num=0
    for small_file in $file/*.txt; do
        for large_file in ${small_file%.txt}/*; do #去掉small_file的.txt后缀
        # echo $small_file
        # echo $large_file
        AST_SMALL_PATH="$small_file"
        AST_LARGE_PATH="$large_file"
        # 去掉small_file的.txt后缀,和/前的内容
        small_file_name=${small_file%.txt}
        small_file_name=${small_file_name##*/}
        # echo $small_file_name
        large_file_name=${large_file%.txt}
        large_file_name=${large_file_name##*/}
        # echo $large_file_name

        OUTPUT_DIR=$REPO_DIR/tmp/output
        SINGLE_OUTPUT_DIR=$BASH_FILE_OUTPUT_PATH/$small_file_name

        mkdir -p $OUTPUT_DIR
        mkdir -p $SINGLE_OUTPUT_DIR
        mkdir -p $SINGLE_OUTPUT_DIR/small
        mkdir -p $SINGLE_OUTPUT_DIR/$large_file_name

        $ATOM_TRANSLATE --lowering-from-AST --mlir-print-debuginfo \
            --realloc-result-path $REALLOC_RESULT_PATH $AST_SMALL_PATH -o $SINGLE_OUTPUT_DIR/small/atom_small.mlir  2>&1 | tee $SINGLE_OUTPUT_DIR/small_translate_log.txt&& 
        $ATOM_TRANSLATE --lowering-from-AST --mlir-print-debuginfo \
            --realloc-result-path $REALLOC_RESULT_PATH $AST_LARGE_PATH -o $SINGLE_OUTPUT_DIR/$large_file_name/atom_large.mlir 2>&1 | tee $SINGLE_OUTPUT_DIR/$large_file_name/large_translate_log.txt && 
            # 如果文件行数大于300就不运行atom_opt
            if [ $(wc -l < ${AST_SMALL_PATH}) -gt 500 ]; then
                echo "文件行数大于500,不运行atom_opt"
            else
                $ATOM_OPT --translation-validation="second-module-path=$SINGLE_OUTPUT_DIR/small base-name-mapping-path=$BASE_NAME_MAPPING_TABLE_PATH" $SINGLE_OUTPUT_DIR/$large_file_name/atom_large.mlir \
                    -o $SINGLE_OUTPUT_DIR/$large_file_name/atom_smt.mlir 2>&1 | tee $SINGLE_OUTPUT_DIR/$large_file_name/large_smt_log.txt 
            fi
            # dot -Tpng $OUTPUT_DIR/smallPDG.dot -o $SINGLE_OUTPUT_DIR/small.png &&
            # dot -Tpng $OUTPUT_DIR/largePDG.dot -o $SINGLE_OUTPUT_DIR/$large_file_name/large.png &&
            dot -Tpng $OUTPUT_DIR/subpdg1.dot -o $SINGLE_OUTPUT_DIR/$large_file_name/subpdg1.png &&
            dot -Tpng $OUTPUT_DIR/subpdg2.dot -o $SINGLE_OUTPUT_DIR/$large_file_name/subpdg2.png &&
            echo "Done"
        single_test_num=$((single_test_num + 1))
        if grep -q "匹配等价" $SINGLE_OUTPUT_DIR/$large_file_name/large_smt_log.txt; then
            single_success_test_num=$((single_success_test_num + 1))
            # 获取软件代码和硬件代码的行数
            software_lines=$(wc -l < "$AST_LARGE_PATH")
            hardware_lines=$(wc -l < "$AST_SMALL_PATH")
            echo "匹配等价:small_file_name: $small_file_name, large_file_name: $large_file_name, 软件代码行数: $software_lines, 硬件代码行数: $hardware_lines" >> $LOG_FILE
        elif grep -q "匹配不等价" $SINGLE_OUTPUT_DIR/$large_file_name/large_smt_log.txt; then
            single_fail_test_num=$((single_fail_test_num + 1))
            # 获取软件代码和硬件代码的行数
            software_lines=$(wc -l < "$AST_LARGE_PATH")
            hardware_lines=$(wc -l < "$AST_SMALL_PATH")
            echo "匹配不等价:small_file_name: $small_file_name, large_file_name: $large_file_name, 软件代码行数: $software_lines, 硬件代码行数: $hardware_lines" >> $LOG_FILE
        elif grep -q "匹配个数: 0" $SINGLE_OUTPUT_DIR/$large_file_name/large_smt_log.txt; then
            single_fail_compare_test_num=$((single_fail_compare_test_num + 1))
        elif grep -q "终态变量不一致" $SINGLE_OUTPUT_DIR/$large_file_name/large_smt_log.txt; then
            single_fail_ending_test_num=$((single_fail_ending_test_num + 1))
            # 获取软件代码和硬件代码的行数
            software_lines=$(wc -l < "$AST_LARGE_PATH")
            hardware_lines=$(wc -l < "$AST_SMALL_PATH")
            echo "终态变量不一致:small_file_name: $small_file_name, large_file_name: $large_file_name, 软件代码行数: $software_lines, 硬件代码行数: $hardware_lines" >> $LOG_FILE
        else
            single_bad_test_num=$((single_bad_test_num + 1))
        fi
        done
    done
    echo "file: $file" >> $LOG_FILE
    echo "该ACC测试数: $single_test_num" >> $LOG_FILE
    echo "该ACC匹配等价数: $single_success_test_num" >> $LOG_FILE
    echo "该ACC匹配不等价数: $single_fail_test_num" >> $LOG_FILE
    echo "该ACC匹配个数为0数: $single_fail_compare_test_num" >> $LOG_FILE
    echo "该ACC匹配终态变量不一致数: $single_fail_ending_test_num" >> $LOG_FILE
    echo "过程中其他问题致中断数: $single_bad_test_num" >> $LOG_FILE
    echo "--------------------------------" >> $LOG_FILE
    total_test_num=$((total_test_num + 1))
    total_num=$((total_num + single_test_num))
    if [ $single_success_test_num -gt 0 ]; then
        total_success_test_num=$((total_success_test_num + single_success_test_num))
        acc_success_test_num=$((acc_success_test_num + 1))
    fi
    if [ $single_fail_test_num -gt 0 ]; then
        total_fail_test_num=$((total_fail_test_num + single_fail_test_num))
        acc_fail_test_num=$((acc_fail_test_num + 1))
    fi
    if [ $single_bad_test_num -gt 0 ]; then
        total_bad_test_num=$((total_bad_test_num + single_bad_test_num))
    fi
    if [ $single_fail_compare_test_num -gt 0 ]; then
        total_fail_compare_test_num=$((total_fail_compare_test_num + single_fail_compare_test_num))
    fi
    if [ $single_fail_ending_test_num -gt 0 ]; then
        total_fail_ending_test_num=$((total_fail_ending_test_num + single_fail_ending_test_num))
        acc_fail_ending_test_num=$((acc_fail_ending_test_num + 1))
    fi
done


echo "总测试ACC数: $total_test_num"
echo "存在匹配等价的ACC数: $acc_success_test_num"
echo "存在匹配不等价的ACC数: $acc_fail_test_num"
echo "存在匹配终态变量不一致的ACC数: $acc_fail_ending_test_num"
echo "总测试数: $total_num"
echo "存在匹配等价的单例数: $total_success_test_num"
echo "存在匹配不等价的单例数: $total_fail_test_num"
echo "存在匹配终态变量不一致的单例数: $total_fail_ending_test_num"
echo "存在匹配个数为0的单例数: $total_fail_compare_test_num"
echo "过程中其他问题致中断数: $total_bad_test_num"

# 写入文件
echo "总测试ACC数: $total_test_num" >> $LOG_FILE
echo "存在匹配等价的ACC数: $acc_success_test_num" >> $LOG_FILE
echo "存在匹配不等价的ACC数: $acc_fail_test_num" >> $LOG_FILE
echo "存在匹配终态变量不一致的ACC数: $acc_fail_ending_test_num" >> $LOG_FILE
echo "总测试数: $total_num" >> $LOG_FILE
echo "存在匹配等价的单例数: $total_success_test_num" >> $LOG_FILE
echo "存在匹配不等价的单例数: $total_fail_test_num" >> $LOG_FILE
echo "存在匹配终态变量不一致的单例数: $total_fail_ending_test_num" >> $LOG_FILE
echo "存在匹配个数为0的单例数: $total_fail_compare_test_num" >> $LOG_FILE
echo "过程中其他问题致中断数: $total_bad_test_num" >> $LOG_FILE
echo "--------------------------------" >> $LOG_FILE