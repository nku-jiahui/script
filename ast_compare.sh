#!/bin/bash
#使用手册
#88ast处理：1.first_extract_between_segId.py 提取指定segid之间的内容
#2. sec_extractFuncs_88.py 提取acc，生成一个文件夹，里面是所有的acc
#680ast处理：1.first_extract_between_segId.py 提取指定segid之间的内容
#2.将所有行向右缩进，，然后执行 process_680ast.py
#然后全体再向右缩进，补充EOF头
#再执行本脚本，生成目标格式文件，最后执行bash_test.sh
#可能会出现文件过多的情况，有时候需要使用split_folder.sh切割一下。

# 微码映射引擎批量调试测试脚本
# 输出格式：
# output/
# ├── single_test1/
# │   ├── hardware1.txt          (硬件AST文件)
# │   └── hardware1/             (硬件文件名文件夹，包含匹配结果)
# │       ├── soft_fuc1.txt
# │       ├── soft_fuc2.txt
# │       └── ...
# ├── single_test2/
# │   ├── hardware2.txt
# │   └── hardware2/
# └── ...

# 设置仓库根目录
REPO_DIR=$(cd $(dirname $BASH_SOURCE[0])/.. && pwd)

echo "=== 微码映射引擎批量调试测试（优化版） ==="

HARDWARE_DIR="$REPO_DIR/data/p_88"
SOFTWARE_FILE="$REPO_DIR/data/simple_680.txt"
SYMBOL_TABLE_FILE="$REPO_DIR/data/simple_680.txt"
OUTPUT_BASE_DIR="$REPO_DIR/tmp/output"
MICROCODE="$REPO_DIR/tmp/build/lib/MicrocodeMapping/microcode-mapping-test"
LOG_FILE="$REPO_DIR/tmp/output/ast-compare.log"

# 创建日志目录
mkdir -p "$(dirname "$LOG_FILE")"

# 清空日志文件并开始记录
echo "=== 微码映射引擎批量调试测试日志 ===" > "$LOG_FILE"
echo "开始时间: $(date)" >> "$LOG_FILE"
echo "" >> "$LOG_FILE"

echo "硬件AST文件夹: $HARDWARE_DIR" | tee -a "$LOG_FILE"
echo "软件AST文件: $SOFTWARE_FILE" | tee -a "$LOG_FILE"
echo "输出基础目录: $OUTPUT_BASE_DIR" | tee -a "$LOG_FILE"
echo "日志文件: $LOG_FILE" | tee -a "$LOG_FILE"

# 检查文件夹是否存在
if [ ! -d "$HARDWARE_DIR" ]; then
    echo "错误: 硬件AST文件夹不存在: $HARDWARE_DIR" | tee -a "$LOG_FILE"
    exit 1
fi

if [ ! -f "$SOFTWARE_FILE" ]; then
    echo "错误: 软件AST文件不存在: $SOFTWARE_FILE" | tee -a "$LOG_FILE"
    exit 1
fi

# 检查可执行文件是否存在
if [ ! -f "$MICROCODE" ]; then
    echo "错误: 微码映射引擎可执行文件不存在: $MICROCODE" | tee -a "$LOG_FILE"
    exit 1
fi

# 创建输出基础目录
# 先删除输出目录中的所有文件
if [ -d "$OUTPUT_BASE_DIR" ]; then
    echo "删除输出目录中的源文件: $OUTPUT_BASE_DIR" | tee -a "$LOG_FILE"
    rm -rf "$OUTPUT_BASE_DIR"/*
fi
mkdir -p "$OUTPUT_BASE_DIR"

echo "" | tee -a "$LOG_FILE"
echo "开始批量处理硬件文件..." | tee -a "$LOG_FILE"

# 分批大小
CHUNK_SIZE=50
TMP_CHUNK_BASE="$OUTPUT_BASE_DIR/.hardware_chunks"

# 收集硬件目录直下的条目（文件或子目录）
entries=()
while IFS= read -r -d '' path; do
    entries+=("$path")
done < <(find "$HARDWARE_DIR" -mindepth 1 -maxdepth 1 \( -type f -o -type d \) -print0)

TOTAL=${#entries[@]}
if [ "$TOTAL" -le "$CHUNK_SIZE" ]; then
    echo "硬件数量: $TOTAL，不超过$CHUNK_SIZE，采用一次性解析" | tee -a "$LOG_FILE"
    if $MICROCODE "$HARDWARE_DIR" "$SOFTWARE_FILE" "$OUTPUT_BASE_DIR" "$SYMBOL_TABLE_FILE" 2>&1 | tee -a "$LOG_FILE"; then
        echo "✓ 批量处理成功完成" | tee -a "$LOG_FILE"
    else
        echo "✗ 批量处理失败" | tee -a "$LOG_FILE"
        exit 1
    fi
else
    # 分批处理
    NUM_CHUNKS=$(( (TOTAL + CHUNK_SIZE - 1) / CHUNK_SIZE ))
    echo "硬件数量: $TOTAL，按每组$CHUNK_SIZE，共 $NUM_CHUNKS 组进行处理" | tee -a "$LOG_FILE"

    # 准备临时分组目录
    rm -rf "$TMP_CHUNK_BASE"
    mkdir -p "$TMP_CHUNK_BASE"

    chunk_idx=0
    start=0
    while [ "$start" -lt "$TOTAL" ]; do
        end=$(( start + CHUNK_SIZE ))
        if [ "$end" -gt "$TOTAL" ]; then
            end=$TOTAL
        fi
        chunk_dir="$TMP_CHUNK_BASE/chunk_$chunk_idx"
        mkdir -p "$chunk_dir"

        # 使用符号链接将本批次的硬件条目映射到chunk目录
        i=$start
        while [ "$i" -lt "$end" ]; do
            base_name=$(basename "${entries[$i]}")
            ln -s "${entries[$i]}" "$chunk_dir/$base_name"
            i=$(( i + 1 ))
        done

        echo "—— 开始处理第 $((chunk_idx+1))/$NUM_CHUNKS 组（索引 $start 到 $((end-1))）——" | tee -a "$LOG_FILE"
        if $MICROCODE "$chunk_dir" "$SOFTWARE_FILE" "$OUTPUT_BASE_DIR" "$SYMBOL_TABLE_FILE" 2>&1 | tee -a "$LOG_FILE"; then
            echo "✓ 第 $((chunk_idx+1)) 组处理完成" | tee -a "$LOG_FILE"
        else
            echo "✗ 第 $((chunk_idx+1)) 组处理失败" | tee -a "$LOG_FILE"
            rm -rf "$TMP_CHUNK_BASE"
            exit 1
        fi

        chunk_idx=$(( chunk_idx + 1 ))
        start=$end
    done

    # 清理临时分组目录
    rm -rf "$TMP_CHUNK_BASE"
    echo "✓ 全部分组处理成功完成" | tee -a "$LOG_FILE"
fi

# 显示输出文件
echo "所有结果保存在: $OUTPUT_BASE_DIR" | tee -a "$LOG_FILE"
echo "输出文件列表:" | tee -a "$LOG_FILE"
ls -la "$OUTPUT_BASE_DIR/" | tee -a "$LOG_FILE"

echo "" | tee -a "$LOG_FILE"
echo "=== 批量处理完成 ===" | tee -a "$LOG_FILE"
echo "结束时间: $(date)" | tee -a "$LOG_FILE"
echo "所有输出文件保存在: $OUTPUT_BASE_DIR" | tee -a "$LOG_FILE"
echo "详细日志保存在: $LOG_FILE" | tee -a "$LOG_FILE" 