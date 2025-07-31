#!/bin/bash

# 微码映射引擎批量调试测试脚本

echo "=== 微码映射引擎批量调试测试 ==="

HARDWARE_DIR="/Users/jiahui/code/huawei-microcode-mapping/data/hardware_files"
SOFTWARE_FILE="/Users/jiahui/code/huawei-microcode-mapping/data/simple_680.txt"
OUTPUT_BASE_DIR="/Users/jiahui/code/huawei-microcode-mapping/tmp/build/tools/microcode-mapping-test/output"
MICROCODE="/Users/jiahui/code/huawei-microcode-mapping/tmp/build/tools/microcode-mapping-test/microcode-mapping-test"

echo "硬件AST文件夹: $HARDWARE_DIR"
echo "软件AST文件: $SOFTWARE_FILE"
echo "输出基础目录: $OUTPUT_BASE_DIR"

# 检查文件夹是否存在
if [ ! -d "$HARDWARE_DIR" ]; then
    echo "错误: 硬件AST文件夹不存在: $HARDWARE_DIR"
    exit 1
fi

if [ ! -f "$SOFTWARE_FILE" ]; then
    echo "错误: 软件AST文件不存在: $SOFTWARE_FILE"
    exit 1
fi

# 检查可执行文件是否存在
if [ ! -f "$MICROCODE" ]; then
    echo "错误: 微码映射引擎可执行文件不存在: $MICROCODE"
    exit 1
fi

# 创建输出基础目录
mkdir -p "$OUTPUT_BASE_DIR"

# 计数器
total_files=0
success_count=0
error_count=0

echo ""
echo "开始批量处理硬件文件..."

# 遍历硬件文件夹中的所有文件
for hardware_file in "$HARDWARE_DIR"/*; do
    # 检查是否是文件
    if [ -f "$hardware_file" ]; then
        total_files=$((total_files + 1))
        
        # 获取文件名（不含路径）
        filename=$(basename "$hardware_file")
        # 获取文件名（不含扩展名）
        basename_no_ext="${filename%.*}"
        
        # 为每个文件创建独立的输出目录
        output_dir="$OUTPUT_BASE_DIR/$basename_no_ext"
        mkdir -p "$output_dir"
        
        echo ""
        echo "处理文件 $total_files: $filename"
        echo "输出目录: $output_dir"
        
        # 运行程序
        if $MICROCODE "$hardware_file" "$SOFTWARE_FILE" "$output_dir"; then
            echo "✓ 成功处理: $filename"
            success_count=$((success_count + 1))
        else
            echo "✗ 处理失败: $filename"
            error_count=$((error_count + 1))
        fi
    fi
done

echo ""
echo "=== 批量处理完成 ==="
echo "总文件数: $total_files"
echo "成功处理: $success_count"
echo "处理失败: $error_count"

# 如果有失败的文件，列出详细信息
if [ $error_count -gt 0 ]; then
    echo ""
    echo "失败的文件列表:"
    for hardware_file in "$HARDWARE_DIR"/*; do
        if [ -f "$hardware_file" ]; then
            filename=$(basename "$hardware_file")
            basename_no_ext="${filename%.*}"
            output_dir="$OUTPUT_BASE_DIR/$basename_no_ext"
            
            # 检查输出目录是否为空或不存在
            if [ ! -d "$output_dir" ] || [ -z "$(ls -A "$output_dir" 2>/dev/null)" ]; then
                echo "  - $filename"
            fi
        fi
    done
fi

echo ""
echo "所有输出文件保存在: $OUTPUT_BASE_DIR" 